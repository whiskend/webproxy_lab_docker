#include <stdio.h>

/** MAX_CACHE_SIZE - 프록시 캐시 전체가 담을 수 있는 최대 바이트 수. */
#define MAX_CACHE_SIZE 1049000
/** MAX_OBJECT_SIZE - 프록시가 캐시에 저장할 단일 객체의 최대 바이트 수. */
#define MAX_OBJECT_SIZE 102400

/** user_agent_hdr - 프록시가 서버에 보낼 고정 User-Agent HTTP 헤더 문자열. */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

/** main - 현재 고정 User-Agent 헤더를 출력하고 정상 종료 시 0을 반환한다. */
int main()
{
  printf("%s", user_agent_hdr);
  return 0;
}
