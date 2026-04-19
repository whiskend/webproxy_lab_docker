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
