#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>
#include <time.h>

#define MAX 4096
#define PORT 1234

int n;

char ans[MAX], buf[MAX];
char line[MAX];
char command[16];
char pathname[64];

struct stat mystat, *sp;
char *t1 = "xwrxwrxwr-------"; //for ls_file
char *t2 = "----------------";

int ls_file(char* file)
{
	struct stat fstat, *sp;
	int r, i; //return and loop values
	char ftime[64]; //filetime
	char temp[MAX];
	
	strcat(line, "\n");
	
	sp = &fstat; //sp->fstat
	
	if( (r=lstat(file, &fstat)) < 0)
		printf("STAT %s FAIL\n", file);
	else
		printf("STAT %s SUCCESS opened %d bits\n", file, sp->st_size); 
				
	//type
	if(S_ISREG(sp->st_mode))
	{
		sprintf(temp, "%c", '-');
		strcat(line, temp);
	}
	if(S_ISDIR(sp->st_mode))
	{
		sprintf(temp, "%c", 'd');
		strcat(line, temp);
	}
	if(S_ISLNK(sp->st_mode))
	{
		sprintf(temp, "%c", 'l');
		strcat(line, temp);
	}		

	//permissions
	for(i=8;i>=0;i--)
	{
		if(sp->st_mode & (1 << i)) //print r|w|x
		{
			sprintf(temp, "%c", t1[i]);
			strcat(line, temp);
		}
		else
		{
			sprintf(temp, "%c", t2[i]);
			strcat(line, temp);
		}
	}
	sprintf(temp, "%4d", sp->st_nlink); //link count
	strcat(line, temp);
	sprintf(temp, "%4d", sp->st_gid); //gid
	strcat(line, temp);
	sprintf(temp, "%4d", sp->st_uid); //uid
	strcat(line, temp);
	sprintf(temp, "%8d", sp->st_size); //file size
	strcat(line, temp);

	//time
	strcpy(ftime, ctime(&sp->st_ctime)); //time in calendar form
	ftime[strlen(ftime)-1] = 0; //kill \n char
	strcat(line, ftime);
	
	//file name
	strcat(line, basename(file));
	//print if link
	if((sp->st_mode & 0xF000) == 0xA000)
	{
		r = readlink(file, buf, MAX);
		sprintf(temp, " -> %s", buf);
		strcat(line, temp);
	}
	strcat(line, "\n");
}

int main() 
{ 
	int sfd, cfd, len; 
	struct sockaddr_in saddr, caddr; 
	int i, length, r, fd;
	
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
	//saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	saddr.sin_port = htons(PORT);

	printf("3. bind socket to server\n");
	if ((bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr))) != 0) { 
		printf("socket bind failed\n"); 
		exit(0); 
	}
		
	getcwd(buf, MAX);
	if((r = chroot(buf)) >= 0)
		printf("4. change virtual root to %s\n", buf);
	else if(r == -1)
		printf("4. change virtual root FAIL\n");
		
	// Now server is ready to listen and verification 
	if ((listen(sfd, 5)) != 0) { 
		printf("Listen failed\n"); 
		exit(0); 
	}
	
	while(1){

		// Try to accept a client connection as descriptor newsock
		length = sizeof(caddr);
		cfd = accept(sfd, (struct sockaddr *)&caddr, &length);
		if (cfd < 0){
			printf("server: accept error\n");
			exit(1);
		}

		printf("server: accepted a client connection from\n");
		printf("-----------------------------------------------\n");
		printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
		printf("-----------------------------------------------\n");

		// Processing loop
		while(1){
			printf("server ready for next request ....\n");
			n = read(cfd, line, MAX);

			//check if client dead
			if (n==0){
				printf("server: client died, server loops\n");
				close(cfd);
				break;
			}

			// show the line string
			printf("server: read  n=%d bytes; line=[%s]\n", n, line);

			//tokenize line into pathname and command
			sscanf(line, "%s %s", command, pathname);
			printf("COMMAND=%s PATHNAME=%s\n", command, pathname);
			
			if(!strcmp(command, "mkdir"))
      {
      	r = mkdir(pathname, 0755);
      	if(!r)
      		printf("MKDIR SUCESS, CREATED %s\n", pathname);
      	else
      		printf("MKDIR FAIL\n");
      }
			else if(!strcmp(command, "rmdir"))
      {
      	r = rmdir(pathname);
      	if(!r)
      		printf("RMDIR SUCESS, RM %s\n", pathname);
      	else
      		printf("RMDIR FAIL\n");
      }
      else if(!strcmp(command, "rm"))
      {
      	r = unlink(pathname);
      	if(!r)
      		printf("RM SUCCESS, RM %s\n", pathname);
      	else
      		printf("RM FAIL\n");
      }
      else if(!strcmp(command, "cd"))
      {
      	r = chdir(pathname);
      	if(!r)
      		printf("CD SUCCESS, CD %s\n", pathname);
      	else
      		printf("CD FAIL\n");
      }
      else if(!strcmp(command, "pwd"))
      {
      	if(!getcwd(buf, MAX))
      		printf("PWD FAIL\n");
      	else
      		printf("CURRENT DIR is: %s\n", buf);
      }
      else if(!strcmp(command, "ls"))
      {
		    memset(line, 0, strlen(line)); //clear return string
        getcwd(buf, MAX);
      	dp = opendir(buf);
      	
      	while(dirp = readdir(dp))
		    	ls_file(dirp->d_name);
      }
      
      if(!strcmp(command, "get"))
      {
			 	memset(line, 0, strlen(line));    	
      	if(!(fd = open(pathname, O_RDONLY)))
      		printf("OPENED %s\n", pathname);
      	while(n = read(fd, line, MAX))
      		r = write(cfd, line, MAX); //write 4096 bits of the file to client
      	
      }
      else if(!strcmp(command, "put"))
      {
      	if((fd = open(pathname, O_WRONLY | O_CREAT)))
      		printf("OPENED %s\n");
      	
      	while(n = read(cfd, ans, MAX))
      		r = write(fd, ans, n);
      	printf("SUCCESSFULY WRITE INTO FILE %s\n", pathname);
      }
      else
      {
				strcat(line, " ECHO");
				
				// send the echo line to client 
				n = write(cfd, line, MAX);
      }
      

			printf("server: wrote n=%d bytes; ECHO = %s\n", n, line);
			printf("server: ready for next request\n");
		}
	}
}


      	
nk
	if((sp->st_mode & 0xF000) == 0xA000)
	{
		r = readlink(file, buf, MAX);
		sprintf(temp, " -> %s", buf);
		strcat(line, temp);
	}
	strcat(line, "\n");
}

int main() 
{ 
	int sfd, cfd, len; 
	struct sockaddr_in saddr, caddr; 
	int i, length, r, fd;
	
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
	//saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	saddr.sin_port = htons(PORT);

	printf("3. bind socket to server\n");
	if ((bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr))) != 0) { 
		printf("socket bind failed\n"); 
		exit(0); 
	}
		
	getcwd(buf, MAX);
	if((r = chroot(buf)) >= 0)
		printf("4. change virtual root to %s\n", buf);
	else if(r == -1)
		printf("4. change virtual root FAIL\n");
		
	// Now server is ready to listen and verification 
	if ((listen(sfd, 5)) != 0) { 
		printf("Listen failed\n"); 
		exit(0); 
	}
	
	while(1){

		// Try to accept a client connection as descriptor newsock
		length = sizeof(caddr);
		cfd = accept(sfd, (struct sockaddr *)&caddr, &length);
		if (cfd < 0){
			printf("server: accept error\n");
			exit(1);
		}

		printf("server: accepted a client connection from\n");
		printf("-----------------------------------------------\n");
		printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
		printf("-----------------------------------------------\n");

		// Processing loop
		while(1){
			printf("server ready for next request ....\n");
			n = read(cfd, line, MAX);

			//check if client dead
			if (n==0){
				printf("server: client died, server loops\n");
				close(cfd);
				break;
			}

			// show the line string
			printf("server: read  n=%d bytes; line=[%s]\n", n, line);

			//tokenize line into pathname and command
			sscanf(line, "%s %s", command, pathname);
			printf("COMMAND=%s PATHNAME=%s\n", command, pathname);
			
			if(!strcmp(command, "mkdir"))
      {
      	r = mkdir(pathname, 0755);
      	if(!r)
      		printf("MKDIR SUCESS, CREATED %s\n", pathname);
      	else
      		printf("MKDIR FAIL\n");
      }
			else if(!strcmp(command, "