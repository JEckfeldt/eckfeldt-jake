// t.c file
#include <stdio.h>
#include <stdlib.h>

int *FP;

int main(int argc, char *argv[], char *env[])
{
  int a,b,c;
  printf("enter main\n");
  
  printf("&argc=%x argv=%x env=%x\n", &argc, argv, env);
  printf("&a=%8x &b=%8x &c=%8x\n", &a, &b, &c);
  
  //write C code to print values of argc and argv[] entries
  
  //print argc
  printf("argc=%d\n", argc);
  
  //print argv[]
  for(int i = 1; i < argc; i++)
  {
    printf("argv[%d]=%s\n", i, argv[i]);
  }
  
  a=1; b=2; c=3;
  A(a, b);
  printf("exit main\n");
}
 
int A(int x, int y)
{
  int d,e,f;
  printf("enter A\n");
   
  //write C code to PRINT ADDRESS OF d, e, f
   
  //print address of d,e,f
  printf("&d=%8x\n&e=%8x\n&f=%8x\n", &d, &e, &f);
  
  d=4; e=5; f=6;
  B(d,e);
  printf("exit A\n");
}
  
int B(int x, int y)
{
  int g,h,i;
  printf("Enter B\n");
    
  //write C code to PRINT ADDRESS OF g,h,i
  
  //print address of g,h,i
  printf("&g=%8x\n&h=%8x\n&i=%8x\n", &g, &h, &i);
    
  g=7; h=8; i=9;
  C(g,h);
  printf("exit B\n");
}

int C(int x, int y)
{
  int u, v, w, i, *p;
  
  printf("enter C\n");
  
  //write C code to PRINT ADDRESS OF u,v,w,i,p;
  
  //print address of u,v,w,i,p
  printf("&u=%8x\n&v=%8x\n&w=%8x\n&i=%8x\n&p=%8x\n", &u, &v, &w, &i, &p);
  
  u=10; v=11; w=12; i=13;
  
  FP = (int *)getebp(); //FP = stack frame ptr of the C() function
  
  //print FP value in hexadecimal
  printf("FP=%8x\n", FP);
  
  // 2) Write C code to print stack frame list
  printf("---Stack Frame List---\n");
  while(FP)
  {
    printf("FP=%8x\n", *FP);
    FP = *FP;
  }
  
  p = (int *)&p;
  
  // 3) print stack contents from p to the frame of main() 
  //just 128 entries
  printf("---Stack Contents---\n");
  p-=4;
  for(int i = 0; i < 128; i++)
  {
    printf("%x=%8x\n", p, *p);
    p++;
  }
  
  
  // 4) on print out, id stack contents as LOCAL VARIABLES, PARAMETERS, stack frame ptr of each function
  
  printf("exit C\n");
}
