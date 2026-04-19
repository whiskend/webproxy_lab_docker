#ifndef __CSAPP_H__
/** __CSAPP_H__ - csapp.h 중복 포함을 막는 include guard 매크로. */
#define __CSAPP_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/** DEF_MODE - 새 파일을 만들 때 사용할 기본 권한 비트이며 umask와 함께 최종 권한을 정한다. */
#define DEF_MODE   S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
/** DEF_UMASK - DEF_MODE에서 그룹/기타 쓰기 권한을 제거하기 위한 기본 umask 값. */
#define DEF_UMASK  S_IWGRP|S_IWOTH

/** SA - bind/connect/accept에 넘길 범용 소켓 주소 구조체 별칭. */
typedef struct sockaddr SA;

/** RIO_BUFSIZE - RIO 내부 읽기 버퍼의 크기(바이트). */
#define RIO_BUFSIZE 8192
/** rio_t - fd, 남은 바이트 수, 다음 위치, 내부 버퍼를 묶은 RIO 버퍼 상태 구조체. */
typedef struct {
    /** rio_fd - 이 RIO 버퍼가 읽을 파일/소켓 디스크립터. */
    int rio_fd;
    /** rio_cnt - 내부 버퍼에 아직 읽지 않고 남아 있는 바이트 수. */
    int rio_cnt;
    /** rio_bufptr - 내부 버퍼에서 다음으로 읽을 바이트 위치. */
    char *rio_bufptr;
    /** rio_buf - 커널 read 결과를 임시로 담는 RIO 내부 버퍼. */
    char rio_buf[RIO_BUFSIZE];
} rio_t;

/** h_errno - 오래된 DNS 함수가 설정하는 전역 DNS 오류 번호. */
extern int h_errno;
/** environ - execve/CGI에 넘길 현재 프로세스 환경 변수 배열. */
extern char **environ;

/** MAXLINE - 텍스트 한 줄이나 작은 문자열 버퍼에 쓰는 최대 바이트 수. */
#define	MAXLINE	 8192
/** MAXBUF - 일반 입출력 버퍼에 쓰는 최대 바이트 수. */
#define MAXBUF   8192
/** LISTENQ - listen의 backlog 인자로 넘기는 대기 연결 큐 크기. */
#define LISTENQ  1024

/** unix_error - msg와 errno 설명을 stderr에 출력하고 프로세스를 종료한다. */
void unix_error(char *msg);
/** posix_error - POSIX 오류 code와 msg를 출력하고 프로세스를 종료한다. */
void posix_error(int code, char *msg);
/** dns_error - 오래된 DNS 조회 오류 msg를 출력하고 프로세스를 종료한다. */
void dns_error(char *msg);
/** gai_error - getaddrinfo/getnameinfo 오류 code와 msg를 출력하고 프로세스를 종료한다. */
void gai_error(int code, char *msg);
/** app_error - 애플리케이션 오류 msg를 출력하고 프로세스를 종료한다. */
void app_error(char *msg);

/** Fork - fork로 자식 프로세스를 만들고 부모에는 자식 PID, 자식에는 0을 반환하며 실패 시 종료한다. */
pid_t Fork(void);
/** Execve - filename 프로그램을 argv/envp로 실행해 현재 프로세스를 교체하며 성공 시 반환하지 않는다. */
void Execve(const char *filename, char *const argv[], char *const envp[]);
/** Wait - 자식 하나가 끝날 때까지 기다리고 status에 종료 상태를 담아 자식 PID를 반환한다. */
pid_t Wait(int *status);
/** Waitpid - pid/options 조건의 자식을 기다리고 iptr에 상태를 담아 대상 PID를 반환한다. */
pid_t Waitpid(pid_t pid, int *iptr, int options);
/** Kill - pid 대상 프로세스/그룹에 signum 시그널을 보내고 실패 시 종료한다. */
void Kill(pid_t pid, int signum);
/** Sleep - secs초 동안 잠들고 시그널로 깨면 남은 초를 반환한다. */
unsigned int Sleep(unsigned int secs);
/** Pause - 시그널이 도착할 때까지 현재 프로세스를 멈춘다. */
void Pause(void);
/** Alarm - seconds초 뒤 SIGALRM을 예약하고 이전 알람의 남은 초를 반환한다. */
unsigned int Alarm(unsigned int seconds);
/** Setpgid - pid 프로세스의 프로세스 그룹을 pgid로 설정한다. */
void Setpgid(pid_t pid, pid_t pgid);
/** Getpgrp - 현재 프로세스의 프로세스 그룹 ID를 반환한다. */
pid_t Getpgrp();

/** handler_t - 시그널 번호 int를 받아 반환값 없이 처리하는 시그널 핸들러 함수 타입. */
typedef void handler_t(int);
/** Signal - signum에 handler를 등록하고 이전 핸들러를 반환한다. */
handler_t *Signal(int signum, handler_t *handler);
/** Sigprocmask - how/set으로 시그널 차단 집합을 바꾸고 oldset에 이전 값을 저장한다. */
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
/** Sigemptyset - set을 아무 시그널도 없는 빈 집합으로 초기화한다. */
void Sigemptyset(sigset_t *set);
/** Sigfillset - set을 모든 시그널을 포함하는 집합으로 초기화한다. */
void Sigfillset(sigset_t *set);
/** Sigaddset - set에 signum 시그널을 추가한다. */
void Sigaddset(sigset_t *set, int signum);
/** Sigdelset - set에서 signum 시그널을 제거한다. */
void Sigdelset(sigset_t *set, int signum);
/** Sigismember - signum이 set에 있으면 1, 없으면 0을 반환한다. */
int Sigismember(const sigset_t *set, int signum);
/** Sigsuspend - 임시 시그널 마스크 set을 적용하고 시그널이 올 때까지 대기한다. */
int Sigsuspend(const sigset_t *set);

/** sio_puts - 문자열 s를 표준 출력에 쓰고 쓴 바이트 수를 반환한다. */
ssize_t sio_puts(char s[]);
/** sio_putl - long 값 v를 10진 문자열로 바꿔 표준 출력에 쓰고 바이트 수를 반환한다. */
ssize_t sio_putl(long v);
/** sio_error - 문자열 s를 시그널 안전 방식으로 출력하고 _exit로 종료한다. */
void sio_error(char s[]);

/** Sio_puts - sio_puts를 오류 처리와 함께 호출하고 쓴 바이트 수를 반환한다. */
ssize_t Sio_puts(char s[]);
/** Sio_putl - sio_putl을 오류 처리와 함께 호출하고 쓴 바이트 수를 반환한다. */
ssize_t Sio_putl(long v);
/** Sio_error - 오류 메시지 s를 출력하고 즉시 종료한다. */
void Sio_error(char s[]);

/** Open - pathname을 flags/mode로 열고 파일 디스크립터를 반환한다. */
int Open(const char *pathname, int flags, mode_t mode);
/** Read - fd에서 count바이트까지 buf로 읽고 실제 읽은 바이트 수를 반환한다. */
ssize_t Read(int fd, void *buf, size_t count);
/** Write - buf의 count바이트를 fd에 쓰고 실제 쓴 바이트 수를 반환한다. */
ssize_t Write(int fd, const void *buf, size_t count);
/** Lseek - fildes의 파일 위치를 whence 기준 offset만큼 옮기고 새 오프셋을 반환한다. */
off_t Lseek(int fildes, off_t offset, int whence);
/** Close - fd 파일 디스크립터를 닫는다. */
void Close(int fd);
/** Select - fd 집합들이 준비될 때까지 timeout만큼 기다리고 준비된 개수를 반환한다. */
int Select(int  n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	   struct timeval *timeout);
/** Dup2 - fd1을 fd2 번호로 복제하고 새 디스크립터 번호를 반환한다. */
int Dup2(int fd1, int fd2);
/** Stat - filename의 파일 정보를 stat 구조체 buf에 채운다. */
void Stat(const char *filename, struct stat *buf);
/** Fstat - fd가 가리키는 파일 정보를 stat 구조체 buf에 채운다. */
void Fstat(int fd, struct stat *buf) ;

/** Opendir - name 디렉터리를 열고 DIR 스트림 포인터를 반환한다. */
DIR *Opendir(const char *name);
/** Readdir - dirp에서 다음 디렉터리 항목을 읽어 dirent 포인터를 반환한다. */
struct dirent *Readdir(DIR *dirp);
/** Closedir - dirp 디렉터리 스트림을 닫고 결과 코드를 반환한다. */
int Closedir(DIR *dirp);

/** Mmap - fd의 offset부터 len바이트를 addr에 매핑하고 시작 주소를 반환한다. */
void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
/** Munmap - start부터 length바이트의 메모리 매핑을 해제한다. */
void Munmap(void *start, size_t length);

/** Fclose - FILE 스트림 fp를 닫는다. */
void Fclose(FILE *fp);
/** Fdopen - fd를 type 모드의 FILE 스트림으로 감싸고 포인터를 반환한다. */
FILE *Fdopen(int fd, const char *type);
/** Fgets - stream에서 최대 n-1문자를 ptr로 읽고 ptr 또는 NULL을 반환한다. */
char *Fgets(char *ptr, int n, FILE *stream);
/** Fopen - filename을 mode로 열고 FILE 스트림 포인터를 반환한다. */
FILE *Fopen(const char *filename, const char *mode);
/** Fputs - 문자열 ptr을 stream에 출력한다. */
void Fputs(const char *ptr, FILE *stream);
/** Fread - stream에서 size바이트 원소 nmemb개를 ptr로 읽고 읽은 원소 수를 반환한다. */
size_t Fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
/** Fwrite - ptr의 size바이트 원소 nmemb개를 stream에 쓴다. */
void Fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/** Malloc - size바이트 힙 메모리를 할당하고 시작 주소를 반환한다. */
void *Malloc(size_t size);
/** Realloc - ptr 블록을 size바이트로 재할당하고 새 시작 주소를 반환한다. */
void *Realloc(void *ptr, size_t size);
/** Calloc - nmemb개 원소(size바이트)를 0으로 초기화해 할당하고 시작 주소를 반환한다. */
void *Calloc(size_t nmemb, size_t size);
/** Free - ptr이 가리키는 힙 메모리 블록을 해제한다. */
void Free(void *ptr);

/** Socket - domain/type/protocol에 맞는 소켓 디스크립터를 생성해 반환한다. */
int Socket(int domain, int type, int protocol);
/** Setsockopt - 소켓 s의 level/optname 옵션을 optval 값으로 설정한다. */
void Setsockopt(int s, int level, int optname, const void *optval, int optlen);
/** Bind - sockfd 소켓을 my_addr 주소와 포트에 연결한다. */
void Bind(int sockfd, struct sockaddr *my_addr, int addrlen);
/** Listen - 소켓 s를 backlog 크기의 연결 대기 소켓으로 전환한다. */
void Listen(int s, int backlog);
/** Accept - 대기 소켓 s에서 연결을 하나 받아 클라이언트 통신용 fd를 반환한다. */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);
/** Connect - sockfd를 serv_addr 서버 주소에 연결한다. */
void Connect(int sockfd, struct sockaddr *serv_addr, int addrlen);

/** Getaddrinfo - node/service 문자열을 hints 조건의 addrinfo 후보 목록 res로 변환한다. */
void Getaddrinfo(const char *node, const char *service,
                 const struct addrinfo *hints, struct addrinfo **res);
/** Getnameinfo - 소켓 주소 sa를 host와 serv 문자열로 변환한다. */
void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host,
                 size_t hostlen, char *serv, size_t servlen, int flags);
/** Freeaddrinfo - getaddrinfo가 만든 addrinfo 목록 res를 해제한다. */
void Freeaddrinfo(struct addrinfo *res);
/** Inet_ntop - 이진 네트워크 주소 src를 출력용 문자열 dst로 변환한다. */
void Inet_ntop(int af, const void *src, char *dst, socklen_t size);
/** Inet_pton - 문자열 주소 src를 이진 네트워크 주소 dst로 변환한다. */
void Inet_pton(int af, const char *src, void *dst);

/** Gethostbyname - 호스트 이름 name의 DNS 정보를 조회해 hostent 포인터를 반환한다. */
struct hostent *Gethostbyname(const char *name);
/** Gethostbyaddr - 이진 주소 addr/len/type으로 DNS 정보를 역조회해 hostent 포인터를 반환한다. */
struct hostent *Gethostbyaddr(const char *addr, int len, int type);

/** Pthread_create - routine(argp)을 실행하는 새 스레드를 만들고 tidp에 ID를 저장한다. */
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
		    void * (*routine)(void *), void *argp);
/** Pthread_join - tid 스레드 종료를 기다리고 반환값을 thread_return에 저장한다. */
void Pthread_join(pthread_t tid, void **thread_return);
/** Pthread_cancel - tid 스레드에 취소 요청을 보낸다. */
void Pthread_cancel(pthread_t tid);
/** Pthread_detach - tid 스레드를 분리해 종료 자원이 자동 회수되게 한다. */
void Pthread_detach(pthread_t tid);
/** Pthread_exit - 현재 스레드를 retval 반환값으로 종료한다. */
void Pthread_exit(void *retval);
/** Pthread_self - 현재 스레드의 pthread_t ID를 반환한다. */
pthread_t Pthread_self(void);
/** Pthread_once - once_control로 init_function을 정확히 한 번만 실행한다. */
void Pthread_once(pthread_once_t *once_control, void (*init_function)());

/** Sem_init - sem 세마포어를 pshared 공유 범위와 value 초기값으로 초기화한다. */
void Sem_init(sem_t *sem, int pshared, unsigned int value);
/** P - sem 값을 감소시키며 0이면 기다리는 wait 연산을 수행한다. */
void P(sem_t *sem);
/** V - sem 값을 증가시켜 기다리는 실행 흐름을 깨울 수 있는 post 연산을 수행한다. */
void V(sem_t *sem);

/** rio_readn - fd에서 n바이트를 usrbuf로 견고하게 읽고 실제 읽은 바이트 수를 반환한다. */
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
/** rio_writen - usrbuf의 n바이트를 fd에 끝까지 쓰고 성공 시 n을 반환한다. */
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
/** rio_readinitb - rp RIO 버퍼를 fd와 연결하고 빈 상태로 초기화한다. */
void rio_readinitb(rio_t *rp, int fd);
/** rio_readnb - rp에서 n바이트를 usrbuf로 버퍼링해 읽고 실제 읽은 바이트 수를 반환한다. */
ssize_t	rio_readnb(rio_t *rp, void *usrbuf, size_t n);
/** rio_readlineb - rp에서 최대 maxlen까지 한 줄을 usrbuf로 읽고 문자 수를 반환한다. */
ssize_t	rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

/** Rio_readn - rio_readn을 오류 처리와 함께 호출해 실제 읽은 바이트 수를 반환한다. */
ssize_t Rio_readn(int fd, void *usrbuf, size_t n);
/** Rio_writen - rio_writen을 오류 처리와 함께 호출해 n바이트를 끝까지 쓴다. */
void Rio_writen(int fd, void *usrbuf, size_t n);
/** Rio_readinitb - rio_readinitb를 호출해 rp를 fd에 연결한다. */
void Rio_readinitb(rio_t *rp, int fd);
/** Rio_readnb - rio_readnb를 오류 처리와 함께 호출해 실제 읽은 바이트 수를 반환한다. */
ssize_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n);
/** Rio_readlineb - rio_readlineb를 오류 처리와 함께 호출해 한 줄의 문자 수를 반환한다. */
ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

/** open_clientfd - hostname:port 서버에 연결하고 연결 fd를 반환하며 실패 시 -1/-2를 반환한다. */
int open_clientfd(char *hostname, char *port);
/** open_listenfd - port에서 연결을 받을 대기 소켓 fd를 만들고 실패 시 -1/-2를 반환한다. */
int open_listenfd(char *port);

/** Open_clientfd - open_clientfd를 오류 처리와 함께 호출하고 연결 fd를 반환한다. */
int Open_clientfd(char *hostname, char *port);
/** Open_listenfd - open_listenfd를 오류 처리와 함께 호출하고 대기 소켓 fd를 반환한다. */
int Open_listenfd(char *port);

#endif
