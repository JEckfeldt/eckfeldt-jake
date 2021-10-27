typedef unsigned int u32;

char *tab = "0123456789ABCDEF";
int  BASE = 10; 

int rpu(u32 x)
{  
    char c;
    if (x){
       c = tab[x % BASE];
       rpu(x / BASE);
       putchar(c);
    }
}

int printu(u32 x)
{
   (x==0)? putchar('0') : rpu(x);
   putchar(' ');
}

//my print decimal function (may be negative)
int printd(int x)
{
  if(x < 0)
  {
    x = x * -1;
    putchar('-');
  }
  printu(x);
}

//my print string function
void prints(char *s)
{
  while(*s != '\0')
  {
    putchar(*s);
    s++;
  }
}

//my print hex function (changes base to 16 and calls printu)
int printx(u32 x)
{
  BASE = 16;
  putchar('0');
  putchar('x');
  printu(x);
  BASE = 10;
}

//my print octal function(change base 8 and call printu)
int printo(u32 x)
{
  BASE = 8;
  putchar('0');
  printu(x);
  BASE = 10;
}

//my PRINTF function
int myprintf(char *fmt, ...)
{
  char *cp = fmt;
  int *ip = (int*)&fmt + 1;
  
  //scan thru cp
  while(*cp) 
  {
    if(*cp == '%')
    {
    cp++;
    
    //check char to see what action we take
      switch(*cp)
      {
        case 'c':
          putchar(*ip);
          break;
         
        case 's':
          prints(*ip);
          break;
          
        case 'u':
          printu(*ip);
          break;
        
        case 'd':
          printd(*ip);
          break;
          
        case 'o': 
          printo(*ip);
          break;
          
        case 'x': 
          printx(*ip);
          break;
      }
      //we used one param so we increment
      //we increment cp because we checked the next char already
      ip++;
    }
    else if(*cp == '\n')
    {
      putchar(*cp);
      putchar('\r');
    }
    else
    {
      putchar(*cp);
    }
    cp++;
  }
}

//**********************MAIN********************
int main(int argc, char *argv[], char *env[])
{
  printf("test myprintf\n");
  myprintf("cha=%c\nstring=%s\nunsigned=%u\nsigned=%d\nhex=%x\noct=%o\n", 'A', "this is test", 123, -123, 123, 123);
  
  myprintf("print argc and argv[]\n");
  myprintf("argc = %d\n", argc);
  for(int i = 0; i < argc; i++)
  {
    myprintf("argv[i] = %s\n", argv[i]);
  }
}
