#include "csapp.h"

/** unix_error - msg와 errno 설명을 stderr에 출력하고 프로세스를 종료한다. */
void unix_error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

/** posix_error - POSIX 오류 code와 msg를 출력하고 프로세스를 종료한다. */
void posix_error(int code, char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(code));
    exit(0);
}

/** gai_error - getaddrinfo/getnameinfo 오류 code와 msg를 출력하고 프로세스를 종료한다. */
void gai_error(int code, char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, gai_strerror(code));
    exit(0);
}

/** app_error - 애플리케이션 오류 msg를 출력하고 프로세스를 종료한다. */
void app_error(char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(0);
}

/** dns_error - 오래된 DNS 조회 오류 msg를 출력하고 프로세스를 종료한다. */
void dns_error(char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(0);
}

/** Fork - fork로 자식 프로세스를 만들고 부모에는 자식 PID, 자식에는 0을 반환하며 실패 시 종료한다. */
pid_t Fork(void)
{
    /** pid - fork로 만든 자식 프로세스 ID를 담는 값. */
    pid_t pid;

    if ((pid = fork()) < 0)
	unix_error("Fork error");
    return pid;
}

/** Execve - filename 프로그램을 argv/envp로 실행해 현재 프로세스를 교체하며 성공 시 반환하지 않는다. */
void Execve(const char *filename, char *const argv[], char *const envp[])
{
    if (execve(filename, argv, envp) < 0)
	unix_error("Execve error");
}

/** Wait - 자식 하나가 끝날 때까지 기다리고 status에 종료 상태를 담아 자식 PID를 반환한다. */
pid_t Wait(int *status)
{
    /** pid - fork로 만든 자식 프로세스 ID를 담는 값. */
    pid_t pid;

    if ((pid  = wait(status)) < 0)
	unix_error("Wait error");
    return pid;
}

/** Waitpid - pid/options 조건의 자식을 기다리고 iptr에 상태를 담아 대상 PID를 반환한다. */
pid_t Waitpid(pid_t pid, int *iptr, int options)
{
    /** retpid - waitpid가 반환한 자식 프로세스 ID. */
    pid_t retpid;

    if ((retpid  = waitpid(pid, iptr, options)) < 0)
	unix_error("Waitpid error");
    return(retpid);
}

/** Kill - pid 대상 프로세스/그룹에 signum 시그널을 보내고 실패 시 종료한다. */
void Kill(pid_t pid, int signum)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = kill(pid, signum)) < 0)
	unix_error("Kill error");
}

/** Pause - 시그널이 도착할 때까지 현재 프로세스를 멈춘다. */
void Pause()
{
    (void)pause();
    return;
}

/** Sleep - secs초 동안 잠들고 시그널로 깨면 남은 초를 반환한다. */
unsigned int Sleep(unsigned int secs)
{
    /** rc - sleep/alarm 계열 호출 결과를 담는 부호 없는 정수. */
    unsigned int rc;

    if ((rc = sleep(secs)) < 0)
	unix_error("Sleep error");
    return rc;
}

/** Alarm - seconds초 뒤 SIGALRM을 예약하고 이전 알람의 남은 초를 반환한다. */
unsigned int Alarm(unsigned int seconds) {
    return alarm(seconds);
}

/** Setpgid - pid 프로세스의 프로세스 그룹을 pgid로 설정한다. */
void Setpgid(pid_t pid, pid_t pgid) {
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = setpgid(pid, pgid)) < 0)
	unix_error("Setpgid error");
    return;
}

/** Getpgrp - 현재 프로세스의 프로세스 그룹 ID를 반환한다. */
pid_t Getpgrp(void) {
    return getpgrp();
}

/** Signal - signum에 handler를 등록하고 이전 핸들러를 반환한다. */
handler_t *Signal(int signum, handler_t *handler)
{
    /** action/old_action - 새 시그널 처리 설정과 이전 설정을 담는 구조체. */
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/** Sigprocmask - how/set으로 시그널 차단 집합을 바꾸고 oldset에 이전 값을 저장한다. */
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    if (sigprocmask(how, set, oldset) < 0)
	unix_error("Sigprocmask error");
    return;
}

/** Sigemptyset - set을 아무 시그널도 없는 빈 집합으로 초기화한다. */
void Sigemptyset(sigset_t *set)
{
    if (sigemptyset(set) < 0)
	unix_error("Sigemptyset error");
    return;
}

/** Sigfillset - set을 모든 시그널을 포함하는 집합으로 초기화한다. */
void Sigfillset(sigset_t *set)
{
    if (sigfillset(set) < 0)
	unix_error("Sigfillset error");
    return;
}

/** Sigaddset - set에 signum 시그널을 추가한다. */
void Sigaddset(sigset_t *set, int signum)
{
    if (sigaddset(set, signum) < 0)
	unix_error("Sigaddset error");
    return;
}

/** Sigdelset - set에서 signum 시그널을 제거한다. */
void Sigdelset(sigset_t *set, int signum)
{
    if (sigdelset(set, signum) < 0)
	unix_error("Sigdelset error");
    return;
}

/** Sigismember - signum이 set에 있으면 1, 없으면 0을 반환한다. */
int Sigismember(const sigset_t *set, int signum)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;
    if ((rc = sigismember(set, signum)) < 0)
	unix_error("Sigismember error");
    return rc;
}

/** Sigsuspend - 임시 시그널 마스크 set을 적용하고 시그널이 올 때까지 대기한다. */
int Sigsuspend(const sigset_t *set)
{
    int rc = sigsuspend(set);
    if (errno != EINTR)
        unix_error("Sigsuspend error");
    return rc;
}

/** sio_reverse - 문자열 s를 제자리에서 뒤집는다. */
static void sio_reverse(char s[])
{
    /** c/i/j - 문자 교환 임시값과 문자열 양끝에서 움직이는 인덱스. */
    int c, i, j;

    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/** sio_ltoa - long 값 v를 b진법 문자열로 변환해 s에 저장한다. */
static void sio_ltoa(long v, char s[], int b)
{
    /** c/i - 현재 자릿값과 결과 문자열에 쓸 위치 인덱스. */
    int c, i = 0;
    /** neg - 변환할 정수 v가 음수인지 기억하는 플래그. */
    int neg = v < 0;

    if (neg)
	v = -v;

    do {
        s[i++] = ((c = (v % b)) < 10)  ?  c + '0' : c - 10 + 'a';
    } while ((v /= b) > 0);

    if (neg)
	s[i++] = '-';

    s[i] = '\0';
    sio_reverse(s);
}

/** sio_strlen - 널 종료 문자열 s의 길이를 반환한다. */
static size_t sio_strlen(char s[])
{
    /** i - 문자열 길이나 현재 위치를 세는 인덱스. */
    int i = 0;

    while (s[i] != '\0')
        ++i;
    return i;
}

/** sio_puts - 문자열 s를 표준 출력에 쓰고 쓴 바이트 수를 반환한다. */
ssize_t sio_puts(char s[])
{
    return write(STDOUT_FILENO, s, sio_strlen(s));
}

/** sio_putl - long 값 v를 10진 문자열로 바꿔 표준 출력에 쓰고 바이트 수를 반환한다. */
ssize_t sio_putl(long v)
{
    /** s - long 값을 문자열로 바꿀 때 쓰는 임시 버퍼. */
    char s[128];

    sio_ltoa(v, s, 10);
    /** sio_puts - 문자열 s를 표준 출력에 쓰고 쓴 바이트 수를 반환한다. */
    return sio_puts(s);
}

/** sio_error - 문자열 s를 시그널 안전 방식으로 출력하고 _exit로 종료한다. */
void sio_error(char s[])
{
    sio_puts(s);
    _exit(1);
}

/** Sio_putl - sio_putl을 오류 처리와 함께 호출하고 쓴 바이트 수를 반환한다. */
ssize_t Sio_putl(long v)
{
    /** n - 실제로 읽거나 쓴 바이트 수를 담는 signed 크기 값. */
    ssize_t n;

    if ((n = sio_putl(v)) < 0)
	sio_error("Sio_putl error");
    return n;
}

/** Sio_puts - sio_puts를 오류 처리와 함께 호출하고 쓴 바이트 수를 반환한다. */
ssize_t Sio_puts(char s[])
{
    /** n - 실제로 읽거나 쓴 바이트 수를 담는 signed 크기 값. */
    ssize_t n;

    if ((n = sio_puts(s)) < 0)
	sio_error("Sio_puts error");
    return n;
}

/** Sio_error - 오류 메시지 s를 출력하고 즉시 종료한다. */
void Sio_error(char s[])
{
    sio_error(s);
}

/** Open - pathname을 flags/mode로 열고 파일 디스크립터를 반환한다. */
int Open(const char *pathname, int flags, mode_t mode)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = open(pathname, flags, mode))  < 0)
	unix_error("Open error");
    return rc;
}

/** Read - fd에서 count바이트까지 buf로 읽고 실제 읽은 바이트 수를 반환한다. */
ssize_t Read(int fd, void *buf, size_t count)
{
    ssize_t rc;

    if ((rc = read(fd, buf, count)) < 0)
	unix_error("Read error");
    return rc;
}

/** Write - buf의 count바이트를 fd에 쓰고 실제 쓴 바이트 수를 반환한다. */
ssize_t Write(int fd, const void *buf, size_t count)
{
    ssize_t rc;

    if ((rc = write(fd, buf, count)) < 0)
	unix_error("Write error");
    return rc;
}

/** Lseek - fildes의 파일 위치를 whence 기준 offset만큼 옮기고 새 오프셋을 반환한다. */
off_t Lseek(int fildes, off_t offset, int whence)
{
    /** rc - lseek가 반환한 새 파일 오프셋. */
    off_t rc;

    if ((rc = lseek(fildes, offset, whence)) < 0)
	unix_error("Lseek error");
    return rc;
}

/** Close - fd 파일 디스크립터를 닫는다. */
void Close(int fd)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = close(fd)) < 0)
	unix_error("Close error");
}

/** Select - fd 집합들이 준비될 때까지 timeout만큼 기다리고 준비된 개수를 반환한다. */
int Select(int  n, fd_set *readfds, fd_set *writefds,
	   fd_set *exceptfds, struct timeval *timeout)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = select(n, readfds, writefds, exceptfds, timeout)) < 0)
	unix_error("Select error");
    return rc;
}

/** Dup2 - fd1을 fd2 번호로 복제하고 새 디스크립터 번호를 반환한다. */
int Dup2(int fd1, int fd2)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = dup2(fd1, fd2)) < 0)
	unix_error("Dup2 error");
    return rc;
}

/** Stat - filename의 파일 정보를 stat 구조체 buf에 채운다. */
void Stat(const char *filename, struct stat *buf)
{
    if (stat(filename, buf) < 0)
	unix_error("Stat error");
}

/** Fstat - fd가 가리키는 파일 정보를 stat 구조체 buf에 채운다. */
void Fstat(int fd, struct stat *buf)
{
    if (fstat(fd, buf) < 0)
	unix_error("Fstat error");
}

/** Opendir - name 디렉터리를 열고 DIR 스트림 포인터를 반환한다. */
DIR *Opendir(const char *name)
{
    /** dirp - opendir로 연 디렉터리 스트림 포인터. */
    DIR *dirp = opendir(name);

    if (!dirp)
        unix_error("opendir error");
    return dirp;
}

/** Readdir - dirp에서 다음 디렉터리 항목을 읽어 dirent 포인터를 반환한다. */
struct dirent *Readdir(DIR *dirp)
{
    /** dep - readdir가 반환한 디렉터리 항목 포인터. */
    struct dirent *dep;

    errno = 0;
    dep = readdir(dirp);
    if ((dep == NULL) && (errno != 0))
        unix_error("readdir error");
    return dep;
}

/** Closedir - dirp 디렉터리 스트림을 닫고 결과 코드를 반환한다. */
int Closedir(DIR *dirp)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = closedir(dirp)) < 0)
        unix_error("closedir error");
    return rc;
}

/** Mmap - fd의 offset부터 len바이트를 addr에 매핑하고 시작 주소를 반환한다. */
void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    /** ptr - mmap이 반환한 매핑 시작 주소. */
    void *ptr;

    if ((ptr = mmap(addr, len, prot, flags, fd, offset)) == ((void *) -1))
	unix_error("mmap error");
    return(ptr);
}

/** Munmap - start부터 length바이트의 메모리 매핑을 해제한다. */
void Munmap(void *start, size_t length)
{
    if (munmap(start, length) < 0)
	unix_error("munmap error");
}

/** Malloc - size바이트 힙 메모리를 할당하고 시작 주소를 반환한다. */
void *Malloc(size_t size)
{
    /** p - 동적 메모리 할당 함수가 반환한 시작 주소. */
    void *p;

    if ((p  = malloc(size)) == NULL)
	unix_error("Malloc error");
    return p;
}

/** Realloc - ptr 블록을 size바이트로 재할당하고 새 시작 주소를 반환한다. */
void *Realloc(void *ptr, size_t size)
{
    /** p - 동적 메모리 할당 함수가 반환한 시작 주소. */
    void *p;

    if ((p  = realloc(ptr, size)) == NULL)
	unix_error("Realloc error");
    return p;
}

/** Calloc - nmemb개 원소(size바이트)를 0으로 초기화해 할당하고 시작 주소를 반환한다. */
void *Calloc(size_t nmemb, size_t size)
{
    /** p - 동적 메모리 할당 함수가 반환한 시작 주소. */
    void *p;

    if ((p = calloc(nmemb, size)) == NULL)
	unix_error("Calloc error");
    return p;
}

/** Free - ptr이 가리키는 힙 메모리 블록을 해제한다. */
void Free(void *ptr)
{
    free(ptr);
}

/** Fclose - FILE 스트림 fp를 닫는다. */
void Fclose(FILE *fp)
{
    if (fclose(fp) != 0)
	unix_error("Fclose error");
}

/** Fdopen - fd를 type 모드의 FILE 스트림으로 감싸고 포인터를 반환한다. */
FILE *Fdopen(int fd, const char *type)
{
    /** fp - 표준 I/O 스트림 포인터. */
    FILE *fp;

    if ((fp = fdopen(fd, type)) == NULL)
	unix_error("Fdopen error");

    return fp;
}

/** Fgets - stream에서 최대 n-1문자를 ptr로 읽고 ptr 또는 NULL을 반환한다. */
char *Fgets(char *ptr, int n, FILE *stream)
{
    /** rptr - fgets가 반환한 버퍼 포인터 또는 NULL. */
    char *rptr;

    if (((rptr = fgets(ptr, n, stream)) == NULL) && ferror(stream))
	app_error("Fgets error");

    return rptr;
}

/** Fopen - filename을 mode로 열고 FILE 스트림 포인터를 반환한다. */
FILE *Fopen(const char *filename, const char *mode)
{
    /** fp - 표준 I/O 스트림 포인터. */
    FILE *fp;

    if ((fp = fopen(filename, mode)) == NULL)
	unix_error("Fopen error");

    return fp;
}

/** Fputs - 문자열 ptr을 stream에 출력한다. */
void Fputs(const char *ptr, FILE *stream)
{
    if (fputs(ptr, stream) == EOF)
	unix_error("Fputs error");
}

/** Fread - stream에서 size바이트 원소 nmemb개를 ptr로 읽고 읽은 원소 수를 반환한다. */
size_t Fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    /** n - 처리한 바이트 수나 원소 수를 담는 부호 없는 크기 값. */
    size_t n;

    if (((n = fread(ptr, size, nmemb, stream)) < nmemb) && ferror(stream))
	unix_error("Fread error");
    return n;
}

/** Fwrite - ptr의 size바이트 원소 nmemb개를 stream에 쓴다. */
void Fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (fwrite(ptr, size, nmemb, stream) < nmemb)
	unix_error("Fwrite error");
}

/** Socket - domain/type/protocol에 맞는 소켓 디스크립터를 생성해 반환한다. */
int Socket(int domain, int type, int protocol)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = socket(domain, type, protocol)) < 0)
	unix_error("Socket error");
    return rc;
}

/** Setsockopt - 소켓 s의 level/optname 옵션을 optval 값으로 설정한다. */
void Setsockopt(int s, int level, int optname, const void *optval, int optlen)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = setsockopt(s, level, optname, optval, optlen)) < 0)
	unix_error("Setsockopt error");
}

/** Bind - sockfd 소켓을 my_addr 주소와 포트에 연결한다. */
void Bind(int sockfd, struct sockaddr *my_addr, int addrlen)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = bind(sockfd, my_addr, addrlen)) < 0)
	unix_error("Bind error");
}

/** Listen - 소켓 s를 backlog 크기의 연결 대기 소켓으로 전환한다. */
void Listen(int s, int backlog)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = listen(s,  backlog)) < 0)
	unix_error("Listen error");
}

/** Accept - 대기 소켓 s에서 연결을 하나 받아 클라이언트 통신용 fd를 반환한다. */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = accept(s, addr, addrlen)) < 0)
	unix_error("Accept error");
    return rc;
}

/** Connect - sockfd를 serv_addr 서버 주소에 연결한다. */
void Connect(int sockfd, struct sockaddr *serv_addr, int addrlen)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = connect(sockfd, serv_addr, addrlen)) < 0)
	unix_error("Connect error");
}

/** Getaddrinfo - node/service 문자열을 hints 조건의 addrinfo 후보 목록 res로 변환한다. */
void Getaddrinfo(const char *node, const char *service,
                 const struct addrinfo *hints, struct addrinfo **res)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = getaddrinfo(node, service, hints, res)) != 0)
        gai_error(rc, "Getaddrinfo error");
}

/** Getnameinfo - 소켓 주소 sa를 host와 serv 문자열로 변환한다. */
void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host,
                 size_t hostlen, char *serv, size_t servlen, int flags)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = getnameinfo(sa, salen, host, hostlen, serv,
                          servlen, flags)) != 0)
        gai_error(rc, "Getnameinfo error");
}

/** Freeaddrinfo - getaddrinfo가 만든 addrinfo 목록 res를 해제한다. */
void Freeaddrinfo(struct addrinfo *res)
{
    freeaddrinfo(res);
}

/** Inet_ntop - 이진 네트워크 주소 src를 출력용 문자열 dst로 변환한다. */
void Inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    if (!inet_ntop(af, src, dst, size))
        unix_error("Inet_ntop error");
}

/** Inet_pton - 문자열 주소 src를 이진 네트워크 주소 dst로 변환한다. */
void Inet_pton(int af, const char *src, void *dst)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    rc = inet_pton(af, src, dst);
    if (rc == 0)
	app_error("inet_pton error: invalid dotted-decimal address");
    else if (rc < 0)
        unix_error("Inet_pton error");
}

/** Gethostbyname - 호스트 이름 name의 DNS 정보를 조회해 hostent 포인터를 반환한다. */
struct hostent *Gethostbyname(const char *name)
{
    /** p - DNS 조회 결과 hostent 구조체 포인터. */
    struct hostent *p;

    if ((p = gethostbyname(name)) == NULL)
	dns_error("Gethostbyname error");
    return p;
}

/** Gethostbyaddr - 이진 주소 addr/len/type으로 DNS 정보를 역조회해 hostent 포인터를 반환한다. */
struct hostent *Gethostbyaddr(const char *addr, int len, int type)
{
    /** p - DNS 조회 결과 hostent 구조체 포인터. */
    struct hostent *p;

    if ((p = gethostbyaddr(addr, len, type)) == NULL)
	dns_error("Gethostbyaddr error");
    return p;
}

/** Pthread_create - routine(argp)을 실행하는 새 스레드를 만들고 tidp에 ID를 저장한다. */
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
		    void * (*routine)(void *), void *argp)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = pthread_create(tidp, attrp, routine, argp)) != 0)
	posix_error(rc, "Pthread_create error");
}

/** Pthread_cancel - tid 스레드에 취소 요청을 보낸다. */
void Pthread_cancel(pthread_t tid) {
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = pthread_cancel(tid)) != 0)
	posix_error(rc, "Pthread_cancel error");
}

/** Pthread_join - tid 스레드 종료를 기다리고 반환값을 thread_return에 저장한다. */
void Pthread_join(pthread_t tid, void **thread_return) {
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = pthread_join(tid, thread_return)) != 0)
	posix_error(rc, "Pthread_join error");
}

/** Pthread_detach - tid 스레드를 분리해 종료 자원이 자동 회수되게 한다. */
void Pthread_detach(pthread_t tid) {
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = pthread_detach(tid)) != 0)
	posix_error(rc, "Pthread_detach error");
}

/** Pthread_exit - 현재 스레드를 retval 반환값으로 종료한다. */
void Pthread_exit(void *retval) {
    pthread_exit(retval);
}

/** Pthread_self - 현재 스레드의 pthread_t ID를 반환한다. */
pthread_t Pthread_self(void) {
    return pthread_self();
}

/** Pthread_once - once_control로 init_function을 정확히 한 번만 실행한다. */
void Pthread_once(pthread_once_t *once_control, void (*init_function)()) {
    pthread_once(once_control, init_function);
}

/** Sem_init - sem 세마포어를 pshared 공유 범위와 value 초기값으로 초기화한다. */
void Sem_init(sem_t *sem, int pshared, unsigned int value)
{
    if (sem_init(sem, pshared, value) < 0)
	unix_error("Sem_init error");
}

/** P - sem 값을 감소시키며 0이면 기다리는 wait 연산을 수행한다. */
void P(sem_t *sem)
{
    if (sem_wait(sem) < 0)
	unix_error("P error");
}

/** V - sem 값을 증가시켜 기다리는 실행 흐름을 깨울 수 있는 post 연산을 수행한다. */
void V(sem_t *sem)
{
    if (sem_post(sem) < 0)
	unix_error("V error");
}

/** rio_readn - fd에서 n바이트를 usrbuf로 견고하게 읽고 실제 읽은 바이트 수를 반환한다. */
ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
    /** nleft - 아직 읽거나 써야 할 남은 바이트 수. */
    size_t nleft = n;
    /** nread - 이번 read/rio_read 호출에서 읽은 바이트 수. */
    ssize_t nread;
    /** bufp - 사용자 버퍼에서 다음으로 읽거나 쓸 위치 포인터. */
    char *bufp = usrbuf;

    while (nleft > 0) {
	if ((nread = read(fd, bufp, nleft)) < 0) {
	    if (errno == EINTR)
		nread = 0;
	    else
		return -1;
	}
	else if (nread == 0)
	    break;
	nleft -= nread;
	bufp += nread;
    }
    return (n - nleft);
}

/** rio_writen - usrbuf의 n바이트를 fd에 끝까지 쓰고 성공 시 n을 반환한다. */
ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    /** nleft - 아직 읽거나 써야 할 남은 바이트 수. */
    size_t nleft = n;
    /** nwritten - 이번 write 호출에서 쓴 바이트 수. */
    ssize_t nwritten;
    /** bufp - 사용자 버퍼에서 다음으로 읽거나 쓸 위치 포인터. */
    char *bufp = usrbuf;

    while (nleft > 0) {
	if ((nwritten = write(fd, bufp, nleft)) <= 0) {
	    if (errno == EINTR)
		nwritten = 0;
	    else
		return -1;
	}
	nleft -= nwritten;
	bufp += nwritten;
    }
    return n;
}

/** rio_read - RIO 내부 버퍼에서 최대 n바이트를 usrbuf로 복사해 복사량을 반환한다. */
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    /** cnt - RIO 내부 버퍼에서 사용자 버퍼로 이번에 복사할 바이트 수. */
    int cnt;

    while (rp->rio_cnt <= 0) {
	rp->rio_cnt = read(rp->rio_fd, rp->rio_buf,
			   sizeof(rp->rio_buf));
	if (rp->rio_cnt < 0) {
	    if (errno != EINTR)
		return -1;
	}
	else if (rp->rio_cnt == 0)
	    return 0;
	else
	    rp->rio_bufptr = rp->rio_buf;
    }

    cnt = n;
    if (rp->rio_cnt < n)
	cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

/** rio_readinitb - rp RIO 버퍼를 fd와 연결하고 빈 상태로 초기화한다. */
void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

/** rio_readnb - rp에서 n바이트를 usrbuf로 버퍼링해 읽고 실제 읽은 바이트 수를 반환한다. */
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
    /** nleft - 아직 읽거나 써야 할 남은 바이트 수. */
    size_t nleft = n;
    /** nread - 이번 read/rio_read 호출에서 읽은 바이트 수. */
    ssize_t nread;
    /** bufp - 사용자 버퍼에서 다음으로 읽거나 쓸 위치 포인터. */
    char *bufp = usrbuf;

    while (nleft > 0) {
	if ((nread = rio_read(rp, bufp, nleft)) < 0)
            return -1;
	else if (nread == 0)
	    break;
	nleft -= nread;
	bufp += nread;
    }
    return (n - nleft);
}

/** rio_readlineb - rp에서 최대 maxlen까지 한 줄을 usrbuf로 읽고 문자 수를 반환한다. */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    /** n/rc - 줄 읽기 위치와 한 글자 읽기 결과 코드. */
    int n, rc;
    /** c/bufp - 방금 읽은 문자와 사용자 버퍼의 다음 저장 위치. */
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) {
        if ((rc = rio_read(rp, &c, 1)) == 1) {
	    *bufp++ = c;
	    if (c == '\n') {
                n++;
     		break;
            }
	} else if (rc == 0) {
	    if (n == 1)
		return 0;
	    else
		break;
	} else
	    return -1;
    }
    *bufp = 0;
    return n-1;
}

/** Rio_readn - rio_readn을 오류 처리와 함께 호출해 실제 읽은 바이트 수를 반환한다. */
ssize_t Rio_readn(int fd, void *ptr, size_t nbytes)
{
    /** n - 실제로 읽거나 쓴 바이트 수를 담는 signed 크기 값. */
    ssize_t n;

    if ((n = rio_readn(fd, ptr, nbytes)) < 0)
	unix_error("Rio_readn error");
    return n;
}

/** Rio_writen - rio_writen을 오류 처리와 함께 호출해 n바이트를 끝까지 쓴다. */
void Rio_writen(int fd, void *usrbuf, size_t n)
{
    if (rio_writen(fd, usrbuf, n) != n)
	unix_error("Rio_writen error");
}

/** Rio_readinitb - rio_readinitb를 호출해 rp를 fd에 연결한다. */
void Rio_readinitb(rio_t *rp, int fd)
{
    rio_readinitb(rp, fd);
}

/** Rio_readnb - rio_readnb를 오류 처리와 함께 호출해 실제 읽은 바이트 수를 반환한다. */
ssize_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
    ssize_t rc;

    if ((rc = rio_readnb(rp, usrbuf, n)) < 0)
	unix_error("Rio_readnb error");
    return rc;
}

/** Rio_readlineb - rio_readlineb를 오류 처리와 함께 호출해 한 줄의 문자 수를 반환한다. */
ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    ssize_t rc;

    if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0)
	unix_error("Rio_readlineb error");
    return rc;
}

/** open_clientfd - hostname:port 서버에 연결하고 연결 fd를 반환하며 실패 시 -1/-2를 반환한다. */
int open_clientfd(char *hostname, char *port) {
    /** clientfd/rc - 서버 연결 소켓 fd와 getaddrinfo 결과 코드. */
    int clientfd, rc;
    /** hints/listp/p - 주소 조회 조건, 후보 목록, 현재 후보 포인터. */
    struct addrinfo hints, *listp, *p;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_flags |= AI_ADDRCONFIG;
    if ((rc = getaddrinfo(hostname, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n", hostname, port, gai_strerror(rc));
        return -2;
    }

    for (p = listp; p; p = p->ai_next) {

        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;

        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
            break;
        if (close(clientfd) < 0) {
            fprintf(stderr, "open_clientfd: close failed: %s\n", strerror(errno));
            return -1;
        }
    }

    freeaddrinfo(listp);
    if (!p)
        return -1;
    else
        return clientfd;
}

/** open_listenfd - port에서 연결을 받을 대기 소켓 fd를 만들고 실패 시 -1/-2를 반환한다. */
int open_listenfd(char *port)
{
    /** hints/listp/p - 주소 조회 조건, 후보 목록, 현재 후보 포인터. */
    struct addrinfo hints, *listp, *p;
    /** listenfd/rc/optval - 대기 소켓 fd, 주소 조회 결과, SO_REUSEADDR 설정값. */
    int listenfd, rc, optval=1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (port %s): %s\n", port, gai_strerror(rc));
        return -2;
    }

    for (p = listp; p; p = p->ai_next) {

        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int));

        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        if (close(listenfd) < 0) {
            fprintf(stderr, "open_listenfd close failed: %s\n", strerror(errno));
            return -1;
        }
    }

    freeaddrinfo(listp);
    if (!p)
        return -1;

    if (listen(listenfd, LISTENQ) < 0) {
        close(listenfd);
	return -1;
    }
    return listenfd;
}

/** Open_clientfd - open_clientfd를 오류 처리와 함께 호출하고 연결 fd를 반환한다. */
int Open_clientfd(char *hostname, char *port)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = open_clientfd(hostname, port)) < 0)
	unix_error("Open_clientfd error");
    return rc;
}

/** Open_listenfd - open_listenfd를 오류 처리와 함께 호출하고 대기 소켓 fd를 반환한다. */
int Open_listenfd(char *port)
{
    /** rc - 시스템/라이브러리 호출의 결과 코드 또는 반환 fd를 임시로 담는 값. */
    int rc;

    if ((rc = open_listenfd(port)) < 0)
	unix_error("Open_listenfd error");
    return rc;
}
