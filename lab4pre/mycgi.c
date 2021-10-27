#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>

#define MAX 10000
#define BLKSIZE 4096

typedef struct {
    char *name;
    char *value;
} ENTRY;

ENTRY entry[MAX];

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int ls(char *dir)
{
	char *dirname = "./";
	DIR *dp;
	struct dirent *dirp;
	
	if(!dir)
	{
		//ls CWD...should just print cwd
		dp = opendir(dirname);
		while((dirp = readdir(dp)) != NULL)
			print_file(dirp->d_name);
		
	}
	else
	{
		//ls dir
	}
	
	return 0;
}

int print_file(char *file)
{
	struct stat fstat;
	struct stat *sp = &fstat;
	int r;
	char ftime[64];
	char link_buf[BLKSIZE];
	
	r = lstat(file, &fstat);
	if((sp->st_mode & 0xF000) == 0x8000) //isreg
		printf("%c", '-');
	else if((sp->st_mode & 0xF000) == 0x4000)
		printf("%c", 'd');
	else if((sp->st_mode & 0xF000) == 0xA000)
		printf("%c", 'l');
		
	for(int i = 0; i< 8; i++)
	{
		if(sp->st_mode & (1 << i))
			printf("%c", t1[i]);
		else
			printf("%c", t2[i]);
	}
	
	printf("%4d ", sp->st_nlink); //link count
	printf("%4d ", sp->st_gid); //gid
	printf("%4d ", sp->st_uid); //uid
	printf("%8d ", sp->st_size); //file size
		
	strcpy(ftime, ctime(&sp->st_ctime));
	ftime[strlen(ftime)-1] = 0;
	printf("%s ", ftime);
	
	printf("%s", basename(file));
	
	if((sp->st_mode & 0xF000) == 0xA000)
	{
		r = readlink(file, link_buf, BLKSIZE);
		printf(" -> %s", link_buf);
	}
	
	printf("\n");
	
	return 0;
}

int cat(char *filename)
{
	int fd = 0;
	int r;
	char buf[BLKSIZE];
	if(!filename)
		printf("<p> No filename given, CAT FAILed<p>");
	else
	{
		fd = open(filename, O_RDONLY);
		if(fd < 0)
			return -1;
			
		//read in file
		while(r = read(fd, buf, BLKSIZE))
			printf("<p> %s <p>", buf);	
	}
	
	return 0;
}

int cp(char *src, char *dest)
{
	char buf[BLKSIZE];
	int fd, gd, n;
	fd = open(src, O_RDONLY);
	gd = open(dest, O_WRONLY | O_CREAT);
	while(n = read(fd, buf, BLKSIZE))
		write(gd, buf, n);
	close(fd);
	close(gd);
		
	return 0;
}


main(int argc, char *argv[]) 
{
  int i, m, r;
  char cwd[128];

  m = getinputs();    // get user inputs name=value into entry[ ]
  getcwd(cwd, 128);   // get CWD pathname

  printf("Content-type: text/html\n\n");
  printf("<p>pid=%d uid=%d cwd=%s\n", getpid(), getuid(), cwd);

  printf("<H1>Echo Your Inputs</H1>");
  printf("You submitted the following name/value pairs:<p>");
 
  for(i=0; i <= m; i++)
     printf("%s = %s<p>", entry[i].name, entry[i].value);
  printf("<p>");


  /*****************************************************************
   Write YOUR C code here to processs the command
         mkdir dirname
         rmdir dirname
         rm    filename
         cat   filename
         cp    file1 file2
         ls    [dirname] <== ls CWD if no dirname
  *****************************************************************/
  
  if(!strcmp(entry[0].value, "mkdir"))
  {
    r = mkdir(entry[1].value, 0766);
    if(r < 0)
    	printf("<p> MKDIR Failed program exit.<p>");
    else
    	printf("<p> Mkdir SUCCESS<p>");
  }
  else if(!strcmp(entry[0].value, "rmdir"))
	{
		r = rmdir(entry[1].value);
		if(r < 0)
			printf("<p> RMDIR Faield program exit<p>");
		else
			printf("<p> RMDIR Success<p>");
	}
	else if(!strcmp(entry[0].value, "rm"))
	{
		r = unlink(entry[1].value);
		if(r < 0)
			printf("<p> RM file FAIL<p>");
		else
			printf("<p> RM Succeed<p>");
	}
	else if(!strcmp(entry[0].value, "cat"))
	{
		r = cat(entry[1].value);
		if(r < 0)
			printf("<p> Cat FAIL, exit program<p>");
		else
			printf("<Cat Succeed<p>");
	}
  else if(!strcmp(entry[0].value, "cp"))
  {
  	r = cp(entry[1].value, entry[2].value);
  	if(r < 0)
  		printf("<p> CP FAIL<p>");
  	else
  		printf("<p> CP Succeed<p>");
  }
  else if(!strcmp(entry[0].value, "ls"))
  {
  	r = ls(0);
  	if(r < 0)
  		printf("LS FAIL");
  }
  
  /************************************************************** 
  // create a FORM webpage for user to submit again 
  printf("</title>");
  printf("</head>");
  printf("<body bgcolor=\"#FF0000\" link=\"#330033\" leftmargin=8 topmargin=8");
  printf("<p>------------------ DO IT AGAIN ----------------\n");
  
  //------ NOTE : CHANGE ACTION to YOUR login name ----------------------------
  //printf("<FORM METHOD=\"POST\" ACTION=\"http://69.166.48.15/~NAME/cgi-bin/mycgi.bin\">");
  
  printf("Enter command : <INPUT NAME=\"command\"> <P>");
  printf("Enter filename1: <INPUT NAME=\"filename1\"> <P>");
  printf("Enter filename2: <INPUT NAME=\"filename2\"> <P>");
  printf("Submit command: <INPUT TYPE=\"submit\" VALUE=\"Click to Submit\"><P>");
  printf("</form>");
  printf("------------------------------------------------<p>");

  printf("</body>");
  printf("</html>");
  ******************************************************/
}
