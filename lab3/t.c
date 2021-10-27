/***** LAB3 base code *****/ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

char gpath[128];    // hold token strings 
char *arg[64];      // token string pointers
int  n;             // number of token strings

char dpath[128];    // hold dir strings in PATH
char *dir[64];      // dir string pointers
int  ndir;          // number of dirs

int out, in, fd; //stores terminal in, out, and  
char *commands[64]; //store commands for pipes
int c; //num commands

char* myPipe()
{
	int pd[2];
	pipe(pd);
	int pid = fork();
	printf("-------PID %s--------\n", pid);
	
	if(pid)
	{
		close(pd[0]);
		close(1);
		dup(pd[1]);
		fd = pd[1];
		close(pd[1]);
		
		return commands[0];
	}
	else
	{
	  close(pd[1]);
	  close(0);
	  dup(pd[0]);
	  fd = pd[0];
	  close(pd[0]);
	  
	  return commands[c-1];  
	}
}

int directOut(char* filename)
{
	fd = open(filename, O_WRONLY|O_CREAT, 0644);
	dup2(fd, 1);
	printf("redirect out >\n");
  return 1;
}

int directAppend(char* filename)
{
	fd = open(filename, O_APPEND|O_CREAT|O_WRONLY, 0644);
	dup2(fd, 1);
	printf("append >>\n");
	return 1;
}

int directIn(char* filename)
{
	FILE *fp;
	char line[128];
	char redirect[2];
	char *s;
	
	fp = fopen(filename, "r");
	fscanf(fp, "%s", line);
	
	printf("redirect in <\n");
	s = strtok(line, " ");
	
	while(s)
	{
		if(n > 0)
		{
			if(strstr("> >> | <", s) == NULL)
			{
				if(!strcmp(redirect, ">"))
					directOut(s);
				else if(!strcmp(redirect, ">>"))
					directAppend(s);
				else if(!strcmp(redirect, "<"))
					directIn(s);
				else
				{
				  printf("arg added at %d, arg: %s\n", n, s);
				  arg[n++] = s;
				  printf("after arg added at %d, arg: %s\n", n-1, arg[n-1]);
				}
			}
			else
			{
				strcpy(redirect, s);
				printf("-----redirect request %s------\n", redirect);
			}
		}
		else
			arg[n++] = s;
		
		s = strtok(0, " ");
	}
	arg[n] = 0;
	return 1;
}
  
  
int tokenize(char *pathname) // YOU have done this in LAB2
{                            // YOU better know how to apply it from now on
  char *s;
  char redirect[2];
  strcpy(gpath, pathname);   // copy into global gpath[]
  s = strtok(gpath, " ");    
  n = 0;
  while(s)
  {
  	printf("-----Tokenizing------\n");
  	if(n > 0)
		{
			if(strstr("> >> | <", s) == NULL)
			{
				if(!strcmp(redirect, ">"))
					directOut(s);
				else if(!strcmp(redirect, ">>"))
					directAppend(s);
				else if(!strcmp(redirect, "<"))
					directIn(s);
				else
				{
				  printf("arg added at %d, arg: %s\n", n, s);
				  arg[n++] = s;
				  printf("after arg added at %d, arg: %s\n", n-1, arg[n-1]);
				}
			}
			else
			{
				strcpy(redirect, s);
				printf("-----redirect request %s------\n", redirect);
			}
		}
		else
			arg[n++] = s;
		
		s = strtok(0, " ");
  }
  arg[n] =0;                // arg[n] = NULL pointer
  for(int i = 0; i < n; i++)
  {
  	printf("-----arg[%d] = %s\n", i, arg[i]);
  } 
  return n;
}

int main(int argc, char *argv[ ], char *env[ ])
{
  int i;
  int pid, status;
  char *cmd;
  char line[28];
  out = dup(1);
  in = dup(0);
  fd = -1;

  // The base code assume only ONE dir[0] -> "/bin"
  dir[0] = "/bin";
  ndir = 1;
  
  for(i=0; i<ndir; i++)
    printf("dir[%d] = %s\n", i, dir[i]);
  
  //processing loop
  while(1){
    printf("sh %d running\n", getpid());
    printf("enter a command line : ");
    fgets(line, 128, stdin);
    line[strlen(line) - 1] = 0; 
    if (line[0]==0)
      continue;
      
    printf("Tokenizing line-------\n");
    if(c < 1)
    {
    	char *s;
    	s = strtok(line, "|");
    	while(s)
    	{
    		printf("commands[%d] = %s\n", c, s);
    		commands[c++] = s;
    		s = strtok(0, "|");
    	}
    }
    else if(c > 1)
    {
    	tokenize(myPipe());
    	for(i = 0; i< c-2; i++)
    	  strcpy(commands[i], commands[i+1]);
    	  
    	commands[c] = 0;
    	commands[c-1] = 0;
    	commands[c-2] = 0;
    	c -= 2;
    }
    else
    {
    	tokenize(commands[0]);
    	commands[c] = 0;
    	c--;
    }
    
    for(i = 0; i < ndir; i++)
    	printf("dir[%d] = %s\n", i, dir[i]);

    cmd = arg[0];         // line = arg0 arg1 arg2 ... 
    

		//simple commands
    if (strcmp(cmd, "cd")==0){
      if(arg[1])
		    chdir(arg[1]);
      else
      	chdir(getenv("HOME"));
      
      continue;
    }
    if (strcmp(cmd, "exit")==0)
      exit(0); 
    
    //fork returns the new pid of the child
     pid = fork();
     
     if (pid){
       printf("sh %d forked a child sh %d\n", getpid(), pid);
       printf("sh %d wait for child sh %d to terminate\n", getpid(), pid);
       
       //sleep parent and restart loop
       pid = wait(&status);
       
       printf("ZOMBIE child=%d exitStatus=%x\n", pid, status); 
       printf("main sh %d repeat loop\n", getpid());
     }
     else{ //no more child running
       printf("child sh %d running\n", getpid());
       
       // make a cmd line = dir[0]/cmd for execve()
       strcpy(line, dir[0]); strcat(line, "/"); strcat(line, cmd);
       printf("line = %s\n", line);
       
       //RECURSIVELY CALLS THE PROGRAM
       //runs the command line, takes list of arguments, environment variables 
       int r = execve(line, arg, env);

       printf("execve failed r = %d\n", r);
       exit(1);
     }
    //-------------------------
    
    if( fd != -1)
    {
    	close(fd);
    	fd = -1;
    	dup2(out, 1);
    	dup2(in, 0);
    	printf("----fd closed----\n");
    }
  }
  
  return 1;
}

/***** LAB3 base code *****/ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

char gpath[128];    // hold token strings 
char *arg[64];      // token string pointers
int  n;             // number of token strings

char dpath[128];    // hold dir strings in PATH
char *dir[64];      // dir string pointers
int  ndir;          // number of dirs

int out, in, fd; //stores terminal in, out, and  
char *commands[64]; //store commands for pipes
int c; //num commands

char* myPipe()
{
	int pd[2];
	pipe(pd);
	int pid = fork();
	printf("-------PID %s--------\n", pid);
	
	if(pid)
	{
		close(pd[0]);
		close(1);
		dup(pd[1]);
		fd = pd[1];
		close(pd[1]);
		
		return commands[0];
	}
	else
	{
	  close(pd[1]);
	  close(0);
	  dup(pd[0]);
	  fd = pd[0];
	  close(pd[0]);
	  
	  return commands[c-1];  
	}
}

int directOut(char* filename)
{
	fd = open(filename, O_WRONLY|O_CREAT, 0644);
	dup2(fd, 1);
	printf("redirect out >\n");
  return 1;
}

int directAppend(char* filename)
{
	fd = open(filename, O_APPEND|O_CREAT|O_WRONLY, 0644);
	dup2(fd, 1);
	printf("append >>\n");
	return 1;
}

int directIn(char* filename)
{
	FILE *fp;
	char line[128];
	char redirect[2];
	char *s;
	
	fp = fopen(filename, "r");
	fscanf(fp, "%s", line);
	
	printf("redirect in <\n");
	s = strtok(line, " ");
	
	while(s)
	{
		if(n > 0)
		{
			if(strstr("> >> | <", s) == NULL)
			{
				if(!strcmp(redirect, ">"))
					directOut(s);
				else if(!strcmp(redirect, ">>"))
					directAppend(s);
				else if(!strcmp(redirect, "<"))
					directIn(s);
				else
				{
				  printf("arg added at %d, arg: %s\n", n, s);
				  arg[n++] = s;
				  printf("after arg added at %d, arg: %s\n", n-1, arg[n-1]);
				}
			}
			else
			{
				strcpy(redirect, s);
				printf("-----redirect request %s------\n", redirect);
			}
		}
		else
			arg[n++] = s;
		
		s = strtok(0, " ");
	}
	arg[n] = 0;
	return 1;
}
  
  
int tokenize(char *pathname) // YOU have done this in LAB2
{                            // YOU better know how to apply it from now on
  char *s;
  char redirect[2];
  strcpy(gpath, pathname);   // copy into global gpath[]
  s = strtok(gpath, " ");    
  n = 0;
  while(s)
  {
  	printf("-----Tokenizing------\n");
  	if(n > 0)
		{
			if(strstr("> >> | <", s) == NULL)
			{
				if(!strcmp(redirect, ">"))
					directOut(s);
				else if(!strcmp(redirect, ">>"))
					directAppend(s);
				else if(!strcmp(redirect, "<"))
					directIn(s);
				else
				{
				  printf("arg added at %d, arg: %s\n", n, s);
				  arg[n++] = s;
				  printf("after arg added at %d, arg: %s\n", n-1, arg[n-1]);
				}
			}
			else
			{
				strcpy(redirect, s);
				printf("-----redirect request %s------\n", redirect);
			}
		}
		else
			arg[n++] = s;
		
		s = strtok(0, " ");
  }
  arg[n] =0;                // arg[n] = NULL pointer
  for(int i = 0; i < n; i++)
  {
  	printf("-----arg[%d] = %s\n", i, arg[i]);
  } 
  return n;
}

int main(int argc, char *argv[ ], char *env[ ])
{
  int i;
  int pid, status;
  char *cmd;
  char line[28];
  out = dup(1);
  in = dup(0);
  fd = -1;

  // The base code assume only ONE dir[0] -> "/bin"
  dir[0] = "/bin";
  ndir = 1;
  
  for(i=0; i<ndir; i++)
    printf("dir[%d] = %s\n", i, dir[i]);
  
  //processing loop
  while(1){
    printf("sh %d running\n", getpid());
    printf("enter a command line : ");
    fgets(line, 128, stdin);
    line[strlen(line) - 1] = 0; 
    if (line[0]==0)
      continue;
      
    printf("Tokenizing line-------\n");
    if(c < 1)
    {
    	char *s;
    	s = strtok(line, "|");
    	while(s)
    	{
    		printf("commands[%d] = %s\n", c, s);
    		commands[c++] = s;
    		s = strtok(0, "|");
    	}
    }
    else if(c > 1)
    {
    	tokenize(myPipe());
    	for(i = 0; i< c-2; i++)
    	  strcpy(commands[i], commands[i+1]);
    	  
    	commands[c] = 0;
    	commands[c-1] = 0;
    	commands[c-2] = 0;
    	c -= 2;
    }
    else
    {
    	tokenize(commands[0]);
    	commands[c] = 0;
    	c--;
    }
    
    for(i = 0; i < ndir; i++)
    	printf("dir[%d] = %s\n", i, dir[i]);

    cmd = arg[0];         // line = arg0 arg1 arg2 ... 
    

		//simple commands
    if (strcmp(cmd, "cd")==0){
      if(arg[1])
		    chdir(arg[1]);
      else
      	chdir(getenv("HOME"));
      
      continue;
    }
    if (strcmp(cmd, "exit")==0)
      exit(0); 
    
    //fork returns the new pid of the child
     pid = fork();
     
     if (pid){
       printf("sh %d forked a child sh %d\n", getpid(), pid);
       printf("sh %d wait for child sh %d to terminate\n", getpid(), pid);
       
       //sleep parent and restart loop
       pid = wait(&status);
       
       printf("ZOMBIE child=%d exitStatus=%x\n", pid, status); 
       printf("main sh %d repeat loop\n", getpid());
     }
     else{ //no more child running
       printf("child sh %d running\n", getpid());
       
       // make a cmd line = dir[0]/cmd for execve()
       strcpy(line, dir[0]); strcat(line, "/"); strcat(line, cmd);
       printf("line = %s\n", line);
       
       //RECURSIVELY CALLS THE PROGRAM
       //runs the command line, takes list of arguments, environment variables 
       int r = execve(line, arg, env);

       printf("execve failed r = %d\n", r);
       exit(1);
     }
    //-------------------------
    
    if( fd != -1)
    {
    	close(fd);
    	fd = -1;
    	dup2(out, 1);
    	dup2(in, 0);
    	printf("----fd closed----\n");
    }
  }
  
  return 1;
}

