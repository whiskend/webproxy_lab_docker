#include "csapp.h"

/*
 * 목표:
 * 클라이언트가 보낸 한 줄을 읽고,
 * 그대로 다시 클라이언트에게 보내는 서버 만들기
 *
 * 참고 절:
 * - 11.1: 클라이언트-서버 프로그래밍 모델
 * - 11.4.9: 에코 클라이언트와 에코 서버 예제
 */

/* echo 함수 선언: 11.4.9의 echo 서버 구조를 참고 */
void echo(int connfd);

int main(int argc, char **argv)
{
    /*
     * 1. 필요한 변수 준비
     *
     * listenfd: 손님을 기다리는 문
     * connfd: 실제 손님 1명과 대화하는 통로
     * clientlen: 클라이언트 주소 구조체의 크기
     * clientaddr: 클라이언트 주소 정보
     * hostname, port: 접속한 클라이언트 정보 출력용
     *
     * 참고 절:
     * - 11.4.1: 소켓 주소 구조체
     * - 11.4.6: accept 함수와 클라이언트 주소 받기
     * - 11.4.7: 호스트/서비스 이름 변환
     * - 11.4.9: 에코 서버 main 함수의 변수 구성
     */
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char hostname[MAXLINE], port[MAXLINE];

    /*
     * 2. 포트 번호가 들어왔는지 확인
     *
     * 실행 예:
     * ./echoserver 8000
     *
     * argc가 2가 아니면 사용법을 출력하고 종료
     *
     * 참고 절:
     * - 11.4.8: open_listenfd 헬퍼 함수는 포트를 문자열로 받음
     * - 11.4.9: 에코 서버 실행 형식
     */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    /*
     * 3. listening socket 열기
     *
     * listenfd = Open_listenfd(argv[1]);
     *
     * 여기까지 성공하면 서버는 해당 포트에서
     * 클라이언트 연결을 기다릴 준비가 된 상태
     *
     * 참고 절:
     * - 11.4.4: bind 함수
     * - 11.4.5: listen 함수
     * - 11.4.8: open_listenfd 헬퍼 함수
     */
    listenfd = open_listenfd(argv[1]);

    /*
     * 4. 서버는 계속 살아 있어야 하므로 무한 루프
     *
     * 참고 절:
     * - 11.1: 서버는 반복적으로 클라이언트 요청을 받음
     * - 11.4.9: 에코 서버의 while 루프 구조
     */
    while (1) {
        /*
         * 5. clientlen 초기화
         *
         * accept를 호출하기 전에
         * clientaddr 구조체 크기를 알려줘야 함
         *
         * 참고 절:
         * - 11.4.1: sockaddr_storage 구조체
         * - 11.4.6: accept의 addrlen 인자
         */
        clientlen = sizeof(clientaddr);

        /*
         * 6. 클라이언트 연결 받기
         *
         * connfd = Accept(listenfd, ...);
         *
         * 여기서 중요한 점:
         * listenfd는 계속 기다리는 문
         * connfd는 이번 클라이언트와 대화하는 통로
         *
         * 참고 절:
         * - 11.4.6: accept 함수
         * - 11.4.9: listenfd와 connfd의 역할 차이
         */
        connfd = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);

        /*
         * 7. 접속한 클라이언트 정보 출력
         *
         * Getnameinfo(...);
         * printf("Connected to ...");
         *
         * 필수 기능은 아니지만,
         * 서버가 연결을 받았는지 확인하기 좋음
         *
         * 참고 절:
         * - 11.4.7: getnameinfo 함수
         * - 11.4.9: 에코 서버가 접속 정보를 출력하는 흐름
         */
        Getnameinfo((struct sockaddr *)&clientaddr, clientlen,
            hostname, MAXLINE,
            port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", hostname, port);

        /*
         * 8. 실제 echo 작업 수행
         *
         * echo(connfd);
         *
         * 이 안에서:
         * 클라이언트가 보낸 줄 읽기
         * 그대로 다시 보내기
         *
         * 참고 절:
         * - 11.4.9: echo 함수 호출 구조
         * - 10.5: RIO 패키지로 안정적으로 읽고 쓰기
         */
        echo(connfd);

        /*
         * 9. 이번 클라이언트와의 연결 닫기
         *
         * Close(connfd);
         *
         * listenfd는 닫지 않음
         * 그래야 다음 클라이언트를 계속 받을 수 있음
         *
         * 참고 절:
         * - 11.4.6: accept가 연결마다 새 connfd를 반환함
         * - 11.4.9: 한 클라이언트 처리가 끝나면 connfd를 닫음
         */
        close(connfd);
    }
}

/*
 * echo 함수
 *
 * 역할:
 * connfd에서 한 줄씩 읽고,
 * 읽은 내용을 그대로 connfd에 다시 쓴다.
 *
 * 참고 절:
 * - 11.4.9: 에코 서버의 echo 함수
 * - 10.5: RIO 패키지
 */
void echo(int connfd)
{
    /*
     * 1. 필요한 변수 준비
     *
     * n: 읽은 바이트 수
     * buf: 클라이언트가 보낸 문자열을 담을 버퍼
     * rio: Robust I/O 상태를 담는 구조체
     *
     * 참고 절:
     * - 10.5.2: RIO의 버퍼링된 입력 함수
     * - 11.4.9: echo 함수의 지역 변수 구성
     */

    /*
     * 2. connfd와 rio 연결
     *
     * Rio_readinitb(&rio, connfd);
     *
     * 이제 rio를 통해 connfd에서 안정적으로 줄 단위 읽기를 할 수 있음
     *
     * 참고 절:
     * - 10.5.2: Rio_readinitb로 RIO 버퍼와 디스크립터 연결
     * - 11.4.9: connected descriptor(connfd)를 RIO에 연결
     */

    /*
     * 3. 클라이언트가 보낸 내용을 한 줄씩 읽기
     *
     * while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
     *
     * n이 0이라는 뜻:
     * 클라이언트가 연결을 닫았다
     * 더 읽을 내용이 없다
     *
     * 참고 절:
     * - 10.5.2: Rio_readlineb로 한 줄씩 읽기
     * - 11.4.9: 클라이언트가 닫을 때까지 반복해서 읽는 echo 함수
     */

        /*
         * 4. 서버 로그 출력
         *
         * printf("server received %d bytes\n", (int)n);
         *
         * 필수는 아니지만,
         * 내가 뭘 받았는지 확인하기 좋음
         *
         * 참고 절:
         * - 11.4.9: 서버가 받은 바이트 수를 출력하는 예제 흐름
         */

        /*
         * 5. 읽은 내용을 그대로 클라이언트에게 다시 보내기
         *
         * Rio_writen(connfd, buf, n);
         *
         * 핵심:
         * 받은 그대로 돌려주기 때문에 echo server
         *
         * 참고 절:
         * - 10.5.1: Rio_writen으로 n바이트를 끝까지 쓰기
         * - 11.4.9: 에코 서버가 받은 데이터를 그대로 돌려주는 핵심 동작
         */

    /*
     * }
     *
     * 반복문이 끝났다는 뜻:
     * 클라이언트가 연결을 종료했다
     * echo 함수가 끝나고 main으로 돌아간다
     *
     * 참고 절:
     * - 11.4.9: echo 함수 종료 후 main에서 connfd를 닫는 흐름
     */
}
