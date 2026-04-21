#include "csapp.h"

/** doit - fd 연결에서 HTTP 요청 하나를 읽어 정적/동적 응답을 보내고 반환한다. */
void doit(int fd);
/** read_requesthdrs - rp에서 빈 줄까지 HTTP 요청 헤더를 읽어 소비한다. */
void read_requesthdrs(rio_t *rp);
/** parse_uri - uri를 filename과 cgiargs로 나누고 정적이면 1, 동적이면 0을 반환한다. */
int parse_uri(char *uri, char *filename, char *cgiargs);
/** serve_static - filename 정적 파일을 HTTP 헤더와 함께 fd 클라이언트에게 전송한다. */
void serve_static(int fd, char *filename, int filesize);
/** get_filetype - filename 확장자를 보고 MIME 타입 문자열을 filetype에 저장한다. */
void get_filetype(char *filename, char *filetype);
/** serve_dynamic - filename CGI 프로그램을 cgiargs와 함께 실행해 fd로 동적 응답을 보낸다. */
void serve_dynamic(int fd, char *filename, char *cgiargs);
/** clienterror - cause/errnum/shortmsg/longmsg로 HTTP 오류 응답을 만들어 fd에 전송한다. */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

/** main - argv[1] 포트로 Tiny 서버를 열고 연결마다 HTTP 트랜잭션을 처리한 뒤 종료 코드를 반환한다. */
int main(int argc, char **argv)
{
  /** listenfd/connfd - listenfd는 연결 대기 소켓, connfd는 클라이언트별 통신 소켓. */
  int listenfd, connfd;
  /** hostname/port - 접속한 클라이언트의 호스트명과 서비스 포트 문자열 버퍼. */
  char hostname[MAXLINE], port[MAXLINE];
  /** clientlen - clientaddr 구조체 크기를 accept/getnameinfo에 알려주는 길이 값. */
  socklen_t clientlen;
  /** clientaddr - IPv4/IPv6 클라이언트 주소를 담는 충분히 큰 주소 구조체. */
  struct sockaddr_storage clientaddr;

  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  // 서버가 사용할 "리스닝 소켓" 생성
// 내부적으로 socket → bind → listen까지 한번에 수행됨
// argv[1] = 포트 번호 (예: "8080")
listenfd = Open_listenfd(argv[1]);

while (1)  // 서버를 계속 살아있게 하는 무한 루프 (연결 계속 받기)
{
    // clientaddr 구조체 크기를 초기화
    // accept()가 이 값을 보고 클라이언트 주소를 채워 넣는다
    clientlen = sizeof(clientaddr);

    // 클라이언트의 connect 요청을 기다림 (블로킹)
    // 요청이 오면 TCP 연결이 완성되고, 그 연결 전용 소켓(connfd) 반환
    // listenfd는 "대기용", connfd는 "통신용"
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

    // 클라이언트의 주소(IP, port)를 사람이 읽을 수 있는 문자열로 변환
    // clientaddr → hostname, port
    Getnameinfo((SA *)&clientaddr, clientlen,
                hostname, MAXLINE,
                port, MAXLINE,
                0);

    // 어떤 클라이언트가 접속했는지 로그 출력
    printf("Accepted connection from (%s, %s)\n", hostname, port);

    // 이 연결(connfd)에 대해 HTTP 요청 1개 처리
    // 내부에서 요청 읽고, 정적/동적 판단하고, 응답까지 보냄
    doit(connfd);

    // 요청 처리 끝났으니 연결 종료
    // (HTTP/1.0이라서 보통 한 요청당 연결 1번 쓰고 끊음)
    Close(connfd);
}
}

/** doit - fd 연결에서 HTTP 요청 하나를 읽어 정적/동적 응답을 보내고 반환한다. */
void doit(int fd)
{
  /*
   * 참고: CSAPP 11.6 Figure 11.30
   *
   * 목표:
   * - 클라이언트가 보낸 HTTP 요청 1개를 처리한다.
   * - 요청 URI가 정적 파일이면 serve_static으로 보낸다.
   * - 요청 URI가 CGI 프로그램이면 serve_dynamic으로 보낸다.
   *
   * 필요한 변수:
   * - is_static: parse_uri 결과를 저장한다. 정적이면 1, 동적이면 0.
   * - sbuf: stat으로 얻은 파일 정보. 파일 종류, 권한, 크기 확인에 사용.
   * - buf: HTTP 요청 줄 원문을 담는 버퍼.
   * - method: GET 같은 HTTP 메서드 문자열.
   * - uri: /home.html, /cgi-bin/adder?1&2 같은 요청 대상.
   * - version: HTTP/1.0 또는 HTTP/1.1 같은 HTTP 버전 문자열.
   * - filename: URI를 서버 파일 경로로 바꾼 결과. 예: ./home.html.
   * - cgiargs: CGI 프로그램에 넘길 인자. 예: 1&2.
   * - rio: fd에서 한 줄씩 안정적으로 읽기 위한 RIO 버퍼.
   *
   * 의사코드:
   * 1. Rio_readinitb(&rio, fd)로 fd와 RIO 버퍼를 연결한다.
   * 2. Rio_readlineb(&rio, buf, MAXLINE)로 HTTP 요청 줄을 읽는다.
   * 3. sscanf(buf, "%s %s %s", method, uri, version)로 요청 줄을 3조각으로 나눈다.
   * 4. method가 GET이 아니면 clienterror로 501 응답을 보내고 return한다.
   * 5. read_requesthdrs(&rio)를 호출해 남은 요청 헤더를 빈 줄까지 읽어 버린다.
   * 6. is_static = parse_uri(uri, filename, cgiargs)로 URI를 파일 경로와 CGI 인자로 나눈다.
   * 7. stat(filename, &sbuf)로 파일이 실제로 있는지 확인한다.
   * 8. 파일이 없으면 clienterror로 404 응답을 보내고 return한다.
   * 9. is_static이 1이면 정적 콘텐츠 처리로 간다.
   * 10. 정적 콘텐츠는 일반 파일인지, 읽기 권한이 있는지 확인한다.
   * 11. 조건이 안 맞으면 clienterror로 403 응답을 보내고 return한다.
   * 12. 조건이 맞으면 serve_static(fd, filename, sbuf.st_size)를 호출한다.
   * 13. is_static이 0이면 동적 콘텐츠 처리로 간다.
   * 14. 동적 콘텐츠는 일반 파일인지, 실행 권한이 있는지 확인한다.
   * 15. 조건이 안 맞으면 clienterror로 403 응답을 보내고 return한다.
   * 16. 조건이 맞으면 serve_dynamic(fd, filename, cgiargs)를 호출한다.
   */
  /** is_static - URI가 정적 콘텐츠면 1, 동적 CGI 콘텐츠면 0인 분기 플래그. */
  int is_static;
  /** sbuf - 요청 파일의 종류/권한/크기 같은 메타데이터를 담는 stat 구조체. */
  struct stat sbuf;
  /** buf/method/uri/version - HTTP 요청 줄 원문과 메서드, URI, 버전 문자열 버퍼. */
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  /** filename/cgiargs - URI에서 만든 로컬 파일 경로와 CGI 인자 문자열 버퍼. */
  char filename[MAXLINE], cgiargs[MAXLINE];
  /** rio - 연결 fd에서 HTTP 텍스트를 안정적으로 읽기 위한 RIO 버퍼 상태. */
  rio_t rio;

  Rio_readinitb(&rio, fd);
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
    return;

  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  if (strcasecmp(method, "GET"))
  {
    clienterror(fd, method, "501", "Not implemented",
                "Tiny does not implement this method");
    return;
  }
  read_requesthdrs(&rio);

  is_static = parse_uri(uri, filename, cgiargs);
  if (stat(filename, &sbuf) < 0)
  {
    clienterror(fd, filename, "404", "Not found",
                "Tiny couldn't find this file");
    return;
  }

  if (is_static)
  {
    if (!S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny couldn't read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size);
  }
  else
  {
    if (!S_ISREG(sbuf.st_mode) || !(S_IXUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny couldn't run the CGI program");
      return;
    }
    serve_dynamic(fd, filename, cgiargs);
  }
}

/** read_requesthdrs - rp에서 빈 줄까지 HTTP 요청 헤더를 읽어 소비한다. */
void read_requesthdrs(rio_t *rp)
{
  /*
   * 참고: CSAPP 11.6 Figure 11.32
   *
   * 목표:
   * - Tiny 기본 버전은 요청 헤더 내용을 사용하지 않는다.
   * - 그래도 헤더가 연결에 남아 있으면 안 되므로 빈 줄까지 읽어서 버린다.
   *
   * 필요한 변수:
   * - buf: 헤더 한 줄을 담는 버퍼.
   *
   * 의사코드:
   * 1. Rio_readlineb(rp, buf, MAXLINE)로 헤더 한 줄을 읽는다.
   * 2. buf가 "\r\n"이 아니면 아직 헤더가 끝나지 않은 것이다.
   * 3. 반복문 안에서 다음 헤더 줄을 계속 읽는다.
   * 4. buf가 "\r\n"이면 헤더 끝을 의미하므로 반복을 멈춘다.
   * 5. 이 함수는 값을 반환하지 않고 끝난다.
   */
  /** buf - HTTP 요청 헤더 한 줄을 담는 임시 버퍼. */
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  while (strcmp(buf, "\r\n"))
  {
    printf("%s", buf);
    Rio_readlineb(rp, buf, MAXLINE);
  }
}

/** parse_uri - uri를 filename과 cgiargs로 나누고 정적이면 1, 동적이면 0을 반환한다. */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
  /*
   * 참고: CSAPP 11.6 Figure 11.33
   *
   * 목표:
   * - 브라우저가 보낸 URI를 서버가 사용할 수 있는 파일 경로로 바꾼다.
   * - 정적 요청과 동적 CGI 요청을 구분한다.
   *
   * Tiny의 규칙:
   * - URI 안에 "cgi-bin"이 없으면 정적 콘텐츠로 본다.
   * - URI 안에 "cgi-bin"이 있으면 동적 콘텐츠로 본다.
   * - 정적 콘텐츠의 기본 시작 위치는 현재 디렉터리 "."이다.
   * - URI가 "/"로 끝나면 기본 파일 "home.html"을 붙인다.
   * - 동적 콘텐츠에서 "?" 뒤쪽은 CGI 인자 cgiargs가 된다.
   *
   * 예시:
   * - "/" -> filename "./home.html", cgiargs "", return 1
   * - "/index.html" -> filename "./index.html", cgiargs "", return 1
   * - "/cgi-bin/adder?1&2" -> filename "./cgi-bin/adder", cgiargs "1&2", return 0
   *
   * 필요한 변수:
   * - ptr: URI 안에서 '?' 위치를 가리키는 포인터.
   *
   * 의사코드:
   * 1. uri 안에 "cgi-bin"이 있는지 검사한다.
   * 2. "cgi-bin"이 없으면 정적 요청이다.
   * 3. 정적 요청에서는 cgiargs를 빈 문자열로 만든다.
   * 4. filename을 "."으로 시작하게 만든다.
   * 5. filename 뒤에 uri를 붙인다.
   * 6. uri의 마지막 글자가 '/'이면 filename 뒤에 "home.html"을 붙인다.
   * 7. 정적 요청이라는 뜻으로 1을 반환한다.
   * 8. "cgi-bin"이 있으면 동적 요청이다.
   * 9. uri 안에서 '?' 위치를 찾는다.
   * 10. '?'가 있으면 '?' 뒤 문자열을 cgiargs에 복사한다.
   * 11. '?' 위치를 '\0'으로 바꿔 uri를 파일 경로 부분까지만 남긴다.
   * 12. '?'가 없으면 cgiargs를 빈 문자열로 만든다.
   * 13. filename을 "."으로 시작하게 만든다.
   * 14. filename 뒤에 '?' 앞까지만 남은 uri를 붙인다.
   * 15. 동적 요청이라는 뜻으로 0을 반환한다.
   */
  /** ptr - 동적 URI에서 물음표 위치를 가리켜 filename과 cgiargs를 나누는 포인터. */
  char *ptr;

  if (!strstr(uri, "cgi-bin"))
  {
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    if (uri[strlen(uri) - 1] == '/')
      strcat(filename, "home.html");
    return 1;
  }

  ptr = strchr(uri, '?');
  if (ptr)
  {
    strcpy(cgiargs, ptr + 1);
    *ptr = '\0';
  }
  else
  {
    strcpy(cgiargs, "");
  }

  strcpy(filename, ".");
  strcat(filename, uri);
  return 0;
}

/** serve_static - filename 정적 파일을 HTTP 헤더와 함께 fd 클라이언트에게 전송한다. */
void serve_static(int fd, char *filename, int filesize)
{
  /*
   * 참고: CSAPP 11.6 Figure 11.34
   *
   * 목표:
   * - 디스크에 이미 존재하는 정적 파일을 HTTP 응답 형태로 클라이언트에게 보낸다.
   * - 응답은 "응답 헤더 + 빈 줄 + 파일 내용" 순서로 전송한다.
   *
   * 필요한 변수:
   * - srcfd: 정적 파일을 열었을 때 얻는 파일 디스크립터.
   * - srcp: mmap으로 메모리에 매핑된 파일 시작 주소.
   * - filetype: Content-type 헤더에 넣을 MIME 타입 문자열.
   * - buf: HTTP 응답 헤더를 조립하는 버퍼.
   *
   * 의사코드:
   * 1. get_filetype(filename, filetype)으로 파일 MIME 타입을 얻는다.
   * 2. buf에 "HTTP/1.0 200 OK\r\n" 응답 줄을 만든다.
   * 3. buf에 "Server: Tiny Web Server\r\n" 헤더를 이어 붙인다.
   * 4. buf에 "Connection: close\r\n" 헤더를 이어 붙인다.
   * 5. buf에 "Content-length: filesize\r\n" 헤더를 이어 붙인다.
   * 6. buf에 "Content-type: filetype\r\n\r\n" 헤더와 빈 줄을 이어 붙인다.
   * 7. Rio_writen(fd, buf, strlen(buf))로 응답 헤더를 클라이언트에게 보낸다.
   * 8. Open(filename, O_RDONLY, 0)로 정적 파일을 연다.
   * 9. Mmap으로 파일 내용을 메모리에 매핑한다.
   * 10. 파일 내용은 메모리에 올라왔으므로 srcfd는 Close로 닫는다.
   * 11. Rio_writen(fd, srcp, filesize)로 파일 내용을 클라이언트에게 보낸다.
   * 12. Munmap(srcp, filesize)로 매핑한 메모리를 해제한다.
   */
  /** srcfd - 정적 파일을 읽기 위해 연 파일 디스크립터. */
  int srcfd;
  /** srcp - mmap으로 메모리에 매핑한 정적 파일의 시작 주소. */
  char *srcp;
  /** filetype - 응답 Content-type 헤더에 넣을 MIME 타입 문자열. */
  char filetype[MAXLINE];
  /** buf - HTTP 응답 헤더를 조립하고 전송하기 위한 버퍼. */
  char buf[MAXBUF];

  get_filetype(filename, filetype);
  snprintf(buf, sizeof(buf),
           "HTTP/1.0 200 OK\r\n"
           "Server: Tiny Web Server\r\n"
           "Connection: close\r\n"
           "Content-length: %d\r\n"
           "Content-type: %s\r\n\r\n",
           filesize, filetype);
  Rio_writen(fd, buf, strlen(buf));

  printf("Response headers:\n");
  printf("%s", buf);

  srcfd = Open(filename, O_RDONLY, 0);
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  Close(srcfd);
  Rio_writen(fd, srcp, filesize);
  Munmap(srcp, filesize);
}

/** get_filetype - filename 확장자를 보고 MIME 타입 문자열을 filetype에 저장한다. */
void get_filetype(char *filename, char *filetype)
{
  /*
   * 참고: CSAPP 11.6 Figure 11.34
   *
   * 목표:
   * - 브라우저가 응답 본문을 어떻게 해석해야 하는지 알려주는 MIME 타입을 정한다.
   *
   * 의사코드:
   * 1. filename 안에 ".html"이 있으면 filetype을 "text/html"로 설정한다.
   * 2. 아니면 ".gif"가 있는지 검사하고 "image/gif"로 설정한다.
   * 3. 아니면 ".png"가 있는지 검사하고 "image/png"로 설정한다.
   * 4. 아니면 ".jpg"가 있는지 검사하고 "image/jpeg"로 설정한다.
   * 5. 위 확장자에 해당하지 않으면 "text/plain"으로 설정한다.
   */
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else
    strcpy(filetype, "text/plain");
}

/** serve_dynamic - filename CGI 프로그램을 cgiargs와 함께 실행해 fd로 동적 응답을 보낸다. */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
  /*
   * 참고: CSAPP 11.6 Figure 11.35
   *
   * 목표:
   * - CGI 프로그램을 실행하고, 그 프로그램의 출력이 클라이언트에게 가게 한다.
   * - 서버는 응답의 앞부분만 보내고, 실제 본문은 CGI 프로그램이 출력한다.
   *
   * 필요한 변수:
   * - buf: HTTP 응답 앞부분을 담는 버퍼.
   * - emptylist: Execve에 넘길 빈 인자 배열.
   *
   * 의사코드:
   * 1. buf에 "HTTP/1.0 200 OK\r\n"를 만들고 Rio_writen으로 보낸다.
   * 2. buf에 "Server: Tiny Web Server\r\n"를 만들고 Rio_writen으로 보낸다.
   * 3. Fork로 자식 프로세스를 만든다.
   * 4. 자식 프로세스라면 setenv("QUERY_STRING", cgiargs, 1)로 CGI 인자를 환경 변수에 넣는다.
   * 5. 자식 프로세스에서 Dup2(fd, STDOUT_FILENO)를 호출한다.
   * 6. 이 Dup2 때문에 CGI 프로그램의 printf 출력이 터미널이 아니라 클라이언트 fd로 간다.
   * 7. 자식 프로세스에서 Execve(filename, emptylist, environ)으로 CGI 프로그램을 실행한다.
   * 8. 부모 프로세스는 Wait(NULL)로 자식이 끝날 때까지 기다리고 자식을 회수한다.
   */
  /** buf - CGI 실행 전에 서버가 먼저 보낼 HTTP 응답 앞부분 버퍼. */
  char buf[MAXLINE];
  /** emptylist - Execve에 넘기는 빈 argv 배열. */
  char *emptylist[] = {NULL};

  snprintf(buf, sizeof(buf), "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  snprintf(buf, sizeof(buf), "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (Fork() == 0)
  {
    setenv("QUERY_STRING", cgiargs, 1);
    Dup2(fd, STDOUT_FILENO);
    Execve(filename, emptylist, environ);
  }
  Wait(NULL);
}

/** clienterror - cause/errnum/shortmsg/longmsg로 HTTP 오류 응답을 만들어 fd에 전송한다. */
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
  /*
   * 참고: CSAPP 11.6 Figure 11.31
   *
   * 목표:
   * - 파일 없음, 권한 없음, 지원하지 않는 메서드 같은 오류도 HTTP 응답 형식으로 보낸다.
   *
   * 필요한 변수:
   * - buf: HTTP 응답 줄과 응답 헤더를 조립하는 버퍼.
   * - body: 브라우저에 보여줄 HTML 오류 본문을 조립하는 버퍼.
   *
   * 인자의 의미:
   * - fd: 오류 응답을 보낼 클라이언트 연결 디스크립터.
   * - cause: 오류를 일으킨 대상. 예: 파일 이름 또는 HTTP 메서드.
   * - errnum: HTTP 상태 코드. 예: "404", "403", "501".
   * - shortmsg: 짧은 상태 메시지. 예: "Not found".
   * - longmsg: 사용자가 읽을 조금 더 자세한 설명.
   *
   * 의사코드:
   * 1. body에 HTML 제목과 body 태그를 만든다.
   * 2. body에 "errnum: shortmsg"를 넣는다.
   * 3. body에 "longmsg: cause"를 넣는다.
   * 4. body 마지막에 Tiny 서버 표시 문구를 넣는다.
   * 5. buf에 "HTTP/1.0 errnum shortmsg\r\n" 응답 줄을 만든다.
   * 6. Rio_writen으로 응답 줄을 보낸다.
   * 7. buf에 "Content-type: text/html\r\n" 헤더를 만들고 보낸다.
   * 8. buf에 "Content-length: strlen(body)\r\n\r\n" 헤더와 빈 줄을 만들고 보낸다.
   * 9. Rio_writen으로 body를 보낸다.
   */
  /** buf - HTTP 오류 응답 줄과 헤더를 조립해 보내는 버퍼. */
  char buf[MAXLINE];
  /** body - 브라우저에 보여줄 HTML 오류 본문을 조립하는 버퍼. */
  char body[MAXBUF];

  snprintf(body, sizeof(body),
           "<html><title>Tiny Error</title>"
           "<body bgcolor=\"ffffff\">\r\n"
           "%s: %s\r\n"
           "<p>%s: %s\r\n"
           "<hr><em>The Tiny Web server</em>\r\n",
           errnum, shortmsg, longmsg, cause);

  snprintf(buf, sizeof(buf), "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  snprintf(buf, sizeof(buf), "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  snprintf(buf, sizeof(buf), "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}
