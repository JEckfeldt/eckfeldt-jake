// t1.c file

#include <stdio.h>
int g;
int main()
{
  static int a,b,c, d[10000];
  a=1; b=2;
  c = a + b;
  printf("c=%d\n", c);
}
