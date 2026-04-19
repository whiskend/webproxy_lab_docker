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

  listenfd = Open_listenfd(argv[1]);
  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);
    Close(connfd);
  }
}

/** doit - fd 연결에서 HTTP 요청 하나를 읽어 정적/동적 응답을 보내고 반환한다. */
void doit(int fd)
{
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
  if (!Rio_readlineb(&rio, buf, MAXLINE))
    return;
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);
  if (strcasecmp(method, "GET"))
  {
    clienterror(fd, method, "501", "Not Implemented",
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
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny couldn't read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size);
  }
  else
  {
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
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
  /** buf - 한 줄 또는 HTTP 헤더 문자열을 담는 임시 버퍼. */
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  printf("%s", buf);
  while (strcmp(buf, "\r\n"))
  {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}

/** parse_uri - uri를 filename과 cgiargs로 나누고 정적이면 1, 동적이면 0을 반환한다. */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
  /** ptr - URI에서 물음표 위치처럼 문자열 내부 위치를 가리키는 포인터. */
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
  else
  {
    ptr = index(uri, '?');
    if (ptr)
    {
      strcpy(cgiargs, ptr + 1);
      *ptr = '\0';
    }
    else
      strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    return 0;
  }
}

/** serve_static - filename 정적 파일을 HTTP 헤더와 함께 fd 클라이언트에게 전송한다. */
void serve_static(int fd, char *filename, int filesize)
{
  /** srcfd - 정적 파일을 읽기 위해 연 파일 디스크립터. */
  int srcfd;
  /** srcp/filetype - mmap된 파일 시작 주소와 응답 Content-Type 문자열 버퍼. */
  char *srcp, filetype[MAXLINE];

  /** buf - HTTP 응답 헤더나 본문을 만들기 위한 큰 임시 버퍼. */
  char buf[MAXBUF];
  /** p - snprintf로 buf에 이어 쓸 현재 위치 포인터. */
  char *p = buf;
  /** n - 방금 읽거나 쓴 바이트 수 또는 snprintf가 만든 문자 수. */
  int n;
  /** remaining - buf에 아직 쓸 수 있는 남은 바이트 수. */
  int remaining = sizeof(buf);

  get_filetype(filename, filetype);

  n = snprintf(p, remaining, "HTTP/1.0 200 OK\r\n");
  p += n;
  remaining -= n;

  n = snprintf(p, remaining, "Server: Tiny Web Server\r\n");
  p += n;
  remaining -= n;

  n = snprintf(p, remaining, "Connection: close\r\n");
  p += n;
  remaining -= n;

  n = snprintf(p, remaining, "Content-length: %d\r\n", filesize);
  p += n;
  remaining -= n;

  n = snprintf(p, remaining, "Content-type: %s\r\n\r\n", filetype);
  p += n;
  remaining -= n;

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
  /** buf/emptylist - 동적 응답 헤더 버퍼와 execve에 넘길 빈 argv 배열. */
  char buf[MAXLINE], *emptylist[] = {NULL};
  /** pid - fork로 만든 자식 프로세스 ID를 담는 값. */
  pid_t pid;

  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if ((pid = Fork()) < 0)
  {
    perror("Fork failed");
    return;
  }

  if (pid == 0)
  {

    setenv("QUERY_STRING", cgiargs, 1);

    if (Dup2(fd, STDOUT_FILENO) < 0)
    {
      perror("Dup2 error");
      exit(1);
    }
    Close(fd);

    Execve(filename, emptylist, environ);

    perror("Execve error");
    exit(1);
  }
  else
  {

    /** status - waitpid가 채우는 자식 프로세스 종료 상태 값. */
    int status;
    if (waitpid(pid, &status, 0) < 0)
    {
      perror("Wait error");
    }

    printf("Child process %d terminated with status %d\n", pid, status);

  }

}

/** clienterror - cause/errnum/shortmsg/longmsg로 HTTP 오류 응답을 만들어 fd에 전송한다. */
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
  /** buf/body - HTTP 오류 응답 헤더와 HTML 본문을 조립하는 버퍼. */
  char buf[MAXLINE], body[MAXBUF];

  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor="
                "ffffff"
                ">\r\n",
          body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}
