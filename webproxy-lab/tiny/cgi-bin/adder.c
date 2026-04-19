#include "csapp.h"

/** main - QUERY_STRING의 두 숫자를 더한 HTML CGI 응답을 stdout으로 출력하고 종료한다. */
int main(void)
{
  /** buf/p - QUERY_STRING 원문과 그 안의 & 구분자 위치 포인터. */
  char *buf, *p;
  /** arg1/arg2/content - CGI 인자 문자열 두 개와 HTML 응답 본문 버퍼. */
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  /** n1/n2 - QUERY_STRING에서 추출한 두 정수 피연산자. */
  int n1 = 0, n2 = 0;

  if ((buf = getenv("QUERY_STRING")) != NULL)
  {
    p = strchr(buf, '&');
    *p = '\0';
    strcpy(arg1, buf);
    strcpy(arg2, p + 1);
    n1 = atoi(strchr(arg1, '=') + 1);
    n2 = atoi(strchr(arg2, '=') + 1);
  }

  sprintf(content, "QUERY_STRING=%s\r\n<p>", buf);
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
