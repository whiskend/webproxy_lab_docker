# Tiny 웹서버 만들기 자료

기준 자료: `CSAPP_2016_11.pdf` 11.4 소켓 인터페이스, 11.5 웹 서버, 11.6 Tiny Web Server.
목표: 책을 보기 전에 Tiny 웹서버의 큰 흐름을 잡고, 직접 구현할 때 무엇을 어디에 써야 하는지 빠르게 확인한다.

## 1. Tiny를 한 문장으로 이해하기

Tiny는 브라우저가 보낸 HTTP 요청 하나를 읽고, 요청한 것이 파일이면 파일을 보내고, CGI 프로그램이면 프로그램을 실행해서 그 출력 결과를 보내는 작은 웹 서버다.

```text
브라우저 요청
  -> main이 연결을 받음
  -> doit이 HTTP 요청 하나를 처리함
  -> parse_uri가 정적/동적 요청을 구분함
  -> serve_static 또는 serve_dynamic이 응답을 보냄
  -> 연결을 닫고 다음 요청을 기다림
```

가장 중요한 질문은 이것이다.

> 이 URI는 정적 파일을 달라는 요청인가, CGI 프로그램을 실행하라는 요청인가?

## 2. 에코 서버에서 Tiny로 넘어가는 연결고리

에코 서버와 Tiny는 뼈대가 같다.

```text
Open_listenfd(port)
while (1) {
    connfd = Accept(listenfd, ...);
    한 클라이언트 처리 함수(connfd);
    Close(connfd);
}
```

에코 서버에서는 `echo(connfd)`가 “받은 줄을 그대로 돌려주는 일”을 한다.
Tiny에서는 `doit(connfd)`가 “HTTP 요청을 해석하고 응답을 보내는 일”을 한다.

즉 Tiny는 에코 서버의 `echo` 자리에 HTTP 처리기 `doit`을 넣은 서버라고 보면 된다.

## 3. listenfd와 connfd

11.4.6의 핵심 개념이다.

- `listenfd`: 서버가 계속 들고 있는 대기용 소켓이다. 클라이언트 연결 요청을 받기 위해 서버 생명 동안 유지된다.
- `connfd`: 클라이언트 한 명과 실제로 읽고 쓰는 연결 소켓이다. `Accept`가 연결마다 새로 만들고, 요청 처리가 끝나면 닫는다.

```text
listenfd = 가게 문
connfd = 들어온 손님 한 명과 대화하는 자리
```

Tiny의 `main`은 이 구조만 담당한다.

```c
listenfd = Open_listenfd(argv[1]);
while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);
    Close(connfd);
}
```

## 4. Tiny 함수 지도

| 함수 | 역할 |
| --- | --- |
| `main` | 포트를 열고, 연결을 받고, `doit`에게 넘기고, 연결을 닫는다. |
| `doit` | HTTP 요청 하나를 처음부터 끝까지 처리한다. |
| `read_requesthdrs` | HTTP 요청 헤더를 빈 줄까지 읽어 소비한다. |
| `parse_uri` | URI를 로컬 파일 이름과 CGI 인자로 나눈다. |
| `serve_static` | 디스크의 정적 파일을 HTTP 응답으로 보낸다. |
| `get_filetype` | 확장자를 보고 MIME 타입을 정한다. |
| `serve_dynamic` | CGI 프로그램을 자식 프로세스로 실행해 동적 응답을 보낸다. |
| `clienterror` | 오류 상황을 HTTP 오류 응답으로 만들어 보낸다. |

## 5. 구현 순서

### 1단계: main 만들기

`main`은 HTTP 내용을 몰라도 된다. 연결을 받고 넘기는 역할만 한다.

체크리스트:

- `argc != 2`이면 사용법 출력 후 종료한다.
- `listenfd = Open_listenfd(argv[1]);`로 대기 소켓을 연다.
- 무한 루프를 돈다.
- `clientlen = sizeof(clientaddr);`로 주소 구조체 크기를 준비한다.
- `connfd = Accept(...)`로 클라이언트 연결을 받는다.
- `Getnameinfo(...)`로 접속한 클라이언트 정보를 문자열로 바꾼다.
- `doit(connfd);`로 HTTP 요청 하나를 처리한다.
- `Close(connfd);`로 이번 클라이언트 연결만 닫는다.

### 2단계: doit 만들기

`doit`은 HTTP 요청 하나를 처리하는 함수다.

흐름:

```text
1. Rio_readinitb(&rio, fd)
2. Rio_readlineb(&rio, buf, MAXLINE)로 요청 줄 읽기
3. sscanf(buf, "%s %s %s", method, uri, version)
4. GET이 아니면 clienterror
5. read_requesthdrs(&rio)로 나머지 헤더 읽기
6. parse_uri(uri, filename, cgiargs)로 정적/동적 구분
7. stat(filename, &sbuf)로 파일 존재 확인
8. 정적이면 읽기 권한 확인 후 serve_static
9. 동적이면 실행 권한 확인 후 serve_dynamic
```

요청 줄 예시:

```text
GET /home.html HTTP/1.1
GET /cgi-bin/adder?1&2 HTTP/1.1
```

Tiny 기본 버전은 `GET`만 처리한다. `POST`, `HEAD`는 과제 확장 문제에서 다룬다.

### 3단계: read_requesthdrs 만들기

HTTP 요청 헤더는 빈 줄로 끝난다. 빈 줄은 C 문자열로 `"\r\n"`이다.

```c
Rio_readlineb(rp, buf, MAXLINE);
while (strcmp(buf, "\r\n")) {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
}
```

Tiny는 헤더 내용을 사용하지 않지만 반드시 읽어서 버린다. 그래야 연결에서 읽기 위치가 정확히 다음 단계로 넘어간다.

### 4단계: parse_uri 만들기

`parse_uri`는 URI를 서버 파일 경로와 CGI 인자로 바꾼다.

정적 요청:

```text
URI: /index.html
filename: ./index.html
cgiargs: ""
return: 1
```

루트 요청:

```text
URI: /
filename: ./home.html
cgiargs: ""
return: 1
```

동적 요청:

```text
URI: /cgi-bin/adder?1&2
filename: ./cgi-bin/adder
cgiargs: 1&2
return: 0
```

기준은 단순하다.

```text
URI에 cgi-bin이 없으면 정적 콘텐츠
URI에 cgi-bin이 있으면 동적 콘텐츠
```

### 5단계: serve_static 만들기

정적 응답은 “HTTP 헤더 + 빈 줄 + 파일 내용”이다.

```text
HTTP/1.0 200 OK\r\n
Server: Tiny Web Server\r\n
Connection: close\r\n
Content-length: 파일크기\r\n
Content-type: MIME타입\r\n
\r\n
파일 내용
```

구현 순서:

```text
1. get_filetype(filename, filetype)
2. 응답 헤더 문자열 만들기
3. Rio_writen(fd, buf, strlen(buf))로 헤더 보내기
4. srcfd = Open(filename, O_RDONLY, 0)
5. srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0)
6. Close(srcfd)
7. Rio_writen(fd, srcp, filesize)로 파일 내용 보내기
8. Munmap(srcp, filesize)
```

`Mmap`을 쓰면 파일 내용을 메모리처럼 다룰 수 있다. 그래서 파일 내용을 직접 반복해서 읽지 않고도 `Rio_writen`으로 보낼 수 있다.

### 6단계: get_filetype 만들기

브라우저는 `Content-type`을 보고 응답을 어떻게 해석할지 결정한다.

```text
.html -> text/html
.gif  -> image/gif
.png  -> image/png
.jpg  -> image/jpeg
그 외 -> text/plain
```

### 7단계: serve_dynamic 만들기

동적 콘텐츠는 서버가 직접 본문을 만들지 않는다. CGI 프로그램을 실행하고, 그 프로그램의 출력이 클라이언트에게 가게 한다.

흐름:

```text
1. 서버가 HTTP/1.0 200 OK와 Server 헤더를 먼저 보낸다.
2. Fork로 자식 프로세스를 만든다.
3. 자식은 setenv("QUERY_STRING", cgiargs, 1)로 CGI 인자를 넘긴다.
4. 자식은 Dup2(fd, STDOUT_FILENO)로 표준 출력을 클라이언트 연결로 바꾼다.
5. 자식은 Execve(filename, emptylist, environ)으로 CGI 프로그램을 실행한다.
6. 부모는 Wait(NULL)로 자식을 회수한다.
```

핵심 한 줄:

```c
Dup2(fd, STDOUT_FILENO);
```

이 줄 때문에 CGI 프로그램의 `printf` 출력이 터미널이 아니라 브라우저로 간다.

### 8단계: clienterror 만들기

없는 파일, 권한 없음, 지원하지 않는 메서드도 HTTP 응답으로 알려줘야 한다.

자주 쓰는 상태 코드:

| 코드 | 의미 |
| --- | --- |
| `200 OK` | 정상 처리 |
| `403 Forbidden` | 파일은 있지만 읽기/실행 권한이 없음 |
| `404 Not found` | 파일이 없음 |
| `501 Not implemented` | 지원하지 않는 HTTP 메서드 |

`clienterror`는 HTML 에러 본문을 만들고, `Content-type`, `Content-length`를 포함한 HTTP 응답을 보낸다.

## 6. 정적 요청과 동적 요청 비교

정적 요청:

```text
GET /home.html HTTP/1.1
parse_uri -> ./home.html, cgiargs="", return 1
serve_static -> 파일 내용 전송
```

동적 요청:

```text
GET /cgi-bin/adder?1&2 HTTP/1.1
parse_uri -> ./cgi-bin/adder, cgiargs="1&2", return 0
serve_dynamic -> Fork, setenv, Dup2, Execve
adder -> printf로 HTML 생성
```

## 7. 직접 구현 체크리스트

`main`:

- [ ] 인자 개수 검사
- [ ] `Open_listenfd`
- [ ] `Accept`
- [ ] `Getnameinfo`
- [ ] `doit`
- [ ] `Close(connfd)`

`doit`:

- [ ] RIO 초기화
- [ ] 요청 줄 읽기
- [ ] `method`, `uri`, `version` 파싱
- [ ] `GET` 검사
- [ ] 헤더 읽기
- [ ] URI 파싱
- [ ] `stat`으로 파일 확인
- [ ] 정적/동적 분기

`parse_uri`:

- [ ] `cgi-bin` 여부 확인
- [ ] 정적 URI를 `./파일명`으로 변환
- [ ] `/` 요청이면 `home.html` 붙이기
- [ ] 동적 URI에서 `?` 뒤를 `cgiargs`로 분리

`serve_static`:

- [ ] MIME 타입 결정
- [ ] 응답 헤더 전송
- [ ] 파일 열기
- [ ] `Mmap`
- [ ] 파일 fd 닫기
- [ ] 파일 내용 전송
- [ ] `Munmap`

`serve_dynamic`:

- [ ] 성공 응답 일부 전송
- [ ] `Fork`
- [ ] 자식에서 `QUERY_STRING` 설정
- [ ] 자식에서 `Dup2`
- [ ] 자식에서 `Execve`
- [ ] 부모에서 `Wait`

## 8. 실행 테스트

Tiny 디렉터리에서:

```bash
cd /Users/igyeong-geun/Documents/jungle/week8/webproxy_lab_docker/webproxy-lab/tiny
make
./tiny 8000
```

정적 콘텐츠 테스트:

```bash
curl -v http://localhost:8000/
curl -v http://localhost:8000/home.html
```

동적 콘텐츠 테스트:

```bash
cd /Users/igyeong-geun/Documents/jungle/week8/webproxy_lab_docker/webproxy-lab/tiny/cgi-bin
make
curl -v "http://localhost:8000/cgi-bin/adder?1&2"
```

만약 직접 만든 CGI가 `num1=1&num2=2` 같은 이름 있는 인자를 받도록 작성되어 있다면 다음처럼 테스트할 수도 있다.

```bash
curl -v "http://localhost:8000/cgi-bin/adder?num1=1&num2=2"
```

## 9. 자주 하는 실수

- 응답 헤더 뒤의 빈 줄 `\r\n`을 빼먹는다.
- `connfd`가 아니라 `listenfd`를 닫는다.
- URI `/`를 `./home.html`로 바꾸지 않는다.
- 동적 URI에서 `?` 뒤 CGI 인자를 분리하지 않는다.
- CGI 실행 전 `Dup2(fd, STDOUT_FILENO)`를 빼먹는다.
- `Mmap` 후 `Munmap`을 빼먹는다.

## 10. 최종 암기 문장

Tiny는 이렇게 외우면 된다.

> `main`은 연결을 받고, `doit`은 요청을 처리하고, `parse_uri`는 정적/동적을 가르고, `serve_static`은 파일을 보내고, `serve_dynamic`은 CGI 출력을 클라이언트에게 보낸다.
