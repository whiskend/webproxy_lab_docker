#include "csapp.h"

/** parse_cgi_int - arg가 "num=1"이면 1, "1"이면 1처럼 CGI 인자 문자열에서 정수를 반환한다. */
static int parse_cgi_int(char *arg);

/** main - QUERY_STRING의 두 숫자를 더한 HTML CGI 응답을 stdout으로 출력하고 종료한다. */
int main(void)
{
  /** buf/p - QUERY_STRING 원문과 그 안의 & 구분자 위치 포인터. */
  char *buf, *p;
  /** query/arg1/arg2/content - 복사한 CGI 인자 원문, 인자 두 개, HTML 응답 본문 버퍼. */
  char query[MAXLINE], arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  /** n1/n2 - QUERY_STRING에서 추출한 두 정수 피연산자. */
  int n1 = 0, n2 = 0;

  if ((buf = getenv("QUERY_STRING")) != NULL)
  {
    snprintf(query, sizeof(query), "%s", buf);
    if ((p = strchr(query, '&')) != NULL)
    {
      *p = '\0';
      strcpy(arg1, query);
      strcpy(arg2, p + 1);
      n1 = parse_cgi_int(arg1);
      n2 = parse_cgi_int(arg2);
    }
  }

  sprintf(content, "QUERY_STRING=%s\r\n<p>", buf ? buf : "");
  sprintf(content + strlen(content), "Welcome to add.com: ");
  sprintf(content + strlen(content), "THE Internet addition portal.\r\n<p>");
  sprintf(content + strlen(content), "The answer is: %d + %d = %d\r\n<p>",
          n1, n2, n1 + n2);
  sprintf(content + strlen(content), "Thanks for visiting!\r\n");

  printf("Content-type: text/html\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("\r\n");
  printf("%s", content);
  fflush(stdout);

  exit(0);
}

/** parse_cgi_int - arg가 "num=1"이면 1, "1"이면 1처럼 CGI 인자 문자열에서 정수를 반환한다. */
static int parse_cgi_int(char *arg)
{
  /** value - 이름 있는 인자에서 '=' 다음 실제 숫자 시작 위치를 가리키는 포인터. */
  char *value = strchr(arg, '=');

  return atoi(value ? value + 1 : arg);
}
