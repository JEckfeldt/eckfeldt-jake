#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h> 
#include <sys/stat.h>
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>     // for dirname()/basename()
#include <time.h> 

#define MAX 4096
#define PORT 1234

char line[MAX], ans[MAX], buf[MAX];
char pathname[64];
char command[16];

struct stat mystat, *sp;
char *t1 = "xwrxwrxwr-------"; //for ls_file
char *t2 = "----------------";

struct sockaddr_in saddr; 
int sfd;

int ls_file(char* file)
{
	struct stat fstat, *sp;
	int r, i; //return and loop values
	char ftime[64]; //filetime
	
	sp = &fstat; //sp->fstat
	
	if( (r=lstat(file, &fstat)) < 0)
		printf("STAT %s FAIL\n", file);
	else
		printf("STAT %s SUCCESS opened %d bits\n", file, sp->st_size); 
				
	//type
	if(S_ISREG(sp->st_mode))
		printf("%c", '-');
	if(S_ISDIR(sp->st_mode))
		printf("%c", 'd');
	if(S_ISLNK(sp->st_mode))
		printf("%c", 'l');		

	//permissions
	for(i=8;i>=0;i--)
	{
		if(sp->st_mode & (1 << i)) //print r|w|x
			printf("%c", t1[i]);
		else
			printf("%c", t2[i]); //print -
	}
	printf("%4d ", sp->st_nlink); //link count
	printf("%4d ", sp->st_gid); //gid 
	printf("%4d ", sp->st_uid); //uid
	printf("%8d ", sp->st_size); //file size
	
	//time
	strcpy(ftime, ctime(&sp->st_ctime)); //time in calendar form
	ftime[strlen(ftime)-1] = 0; //kill \n char
	printf("%s ", ftime);
	
	//file name
	printf("%s", basename(file));
	//print if link
	if((sp->st_mode & 0xF000) == 0xA000)
	{
		r = readlink(file, buf, MAX);
		printf(" -> %s", buf);
	}
	
	printf("\n");
}

int main(int argc, char *argv[], char *env[]) 
{ 
    int n; char how[64];
    int i, r, fd;
    //ls variables
    DIR *dp;
    struct dirent *dirp;

    printf("1. create a socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sfd < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    saddr.sin_port = htons(PORT); 
  
    printf("3. connect to server\n");
    if (connect(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 

    printf("********  processing loop  *********\n");
    while (1){
      printf("input a line : ");
      bzero(line, MAX);                // zero out line[ ]
      fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

      line[strlen(line)-1] = 0;        // kill \n at end
      if (line[0]==0)                  // exit if NULL line
         exit(0);
      //tokenize line into pathname and command
    	sscanf(line, "%s %s", command, pathname);
      printf("command=%s pathname=%s\n", command, pathname);
      
      printf("Checking for LOCAL COMMAND\n");
      
      if(!strcmp(command, "lmkdir"))
      {
      	r = mkdir(pathname, 0755);
      	if(!r)
      		printf("Lmkdir SUCESS, CREATED %s\n", pathname);
      	else
      		printf("Lmkdir FAIL\n");
      }
      else if(!strcmp(command, "lrmdir"))
      {
      	r = rmdir(pathname);
      	if(!r)
      		printf("Lrmdir SUCESS, RM %s\n", pathname);
      	else
      		printf("Lrmdir FAIL\n");
      }
      else if(!strcmp(command, "lrm"))
      {
      	r = unlink(pathname);
      	if(!r)
      		printf("Lrm SUCCESS, RM %s\n", pathname);
      	else
      		printf("Lrm FAIL\n");
      }
      else if(!strcmp(command, "lcd"))
      {
      	r = chdir(pathname);
      	if(!r)
      		printf("Lcd SUCCESS, CD %s\n", pathname);
      	else
      		printf("Lcd FAIL\n");
      }
      else if(!strcmp(command, "lpwd"))
      {
      	if(!getcwd(buf, MAX))
      		printf("Lpwd FAIL\n");
      	else
      		printf("Current DIR is: %s\n", buf);
      }
      else if(!strcmp(command, "lcat"))
      {
      	if((fd = open(pathname, O_RDONLY)) >= 0)
      	{
		  		while(n = read(fd, buf, MAX))
		  		{
		  			r = write(1, buf, n);
		  			if(r < 0)
		  				printf("WRITE FAIL\n");
		  			else
		  				printf("WROTE %d bits\n", n);	
		  		}
      	}
      	else
      		printf("Lcat FAIL\n");
      }
      else if(!strcmp(command, "lls"))
      {
      	getcwd(buf, MAX);
      	dp = opendir(buf);
      	
      	while(dirp = readdir(dp))
      		ls_file(dirp->d_name);
      }
      
      
	    // Send ENTIRE line to server
	    n = write(sfd, line, MAX);
	    printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

      if(!strcmp(command, "get"))
      {
      	if((fd = open(pathname, O_WRONLY | O_CREAT)))
      		printf("OPENED %s\n");
      	
      	while(n = read(sfd, ans, MAX)) //while(n, n-=r) where n is file size and r is bits read 
      		r = write(fd, ans, n);
      	printf("SUCCESSFULY WRITE INTO FILE %s\n", pathname);
      }
      else if(!strcmp(command, "put"))
      {
			 	memset(line, 0, strlen(line));    	
      	if(!(fd = open(pathname, O_RDONLY)))
      		printf("OPENED %s\n", pathname);
      	while(n = read(fd, line, MAX))
      		r = write(sfd, line, MAX); //write 4096 bits of the file to client
      	
      }
      else
      {
			  // Read a line from sock and show it
			  n = read(sfd, ans, MAX);
			  printf("client: read  n=%d bytes;\n",n);
			  printf("ECHO----\n");
			  printf("%s\n", ans);      
      }
      
  }
}


