#include <stdio.h>             // for I/O
#include <stdlib.h>            // for I/O
#include <libgen.h>            // for dirname()/basename()
#include <string.h>

typedef struct node{
         char  name[64];       // node's name string
         char  type;           // 'D' for DIR; 'F' for file
   struct node *child, *sibling, *parent;
}NODE;

//global variables
NODE *root, *cwd, *start;
char line[128];
char command[16], pathname[64];
char dname[64], bname[64]; //for dir_name and base_name

char *cmd[] = {"mkdir", "ls", "quit", "cd", "pwd", "rmdir", "rm", "creat", "save", "reload", 0};

int findCmd(char *command)
{
   int i = 0;
   while(cmd[i]){
     if (strcmp(command, cmd[i])==0)
         return i;
     i++;
   }
   return -1;
}

NODE *search_child(NODE *parent, char *name)
{
  NODE *p;
  printf("search for %s in parent DIR\n", name);
  p = parent->child;
  if (p==0)
    return 0;
  while(p){
    if (strcmp(p->name, name)==0)
      return p;
    p = p->sibling;
  }
  return 0;
}

int insert_child(NODE *parent, NODE *q)
{
  NODE *p;
  printf("insert NODE %s into END of parent child list\n", q->name);
  p = parent->child;
  if (p==0)
    parent->child = q;
  else{
    while(p->sibling)
      p = p->sibling;
    p->sibling = q;
  }
  q->parent = parent;
  q->child = 0;
  q->sibling = 0;
}


//sets dirname to dname and basename to bname
int dbname(char *pathname)
{
  char temp[128]; //dirname/basename destroy pathname
  strcpy(temp, pathname);
  strcpy(dname, dirname(temp));
  strcpy(temp, pathname);
  strcpy(bname, basename(temp));
}

/***************************************************************
 This mkdir(char *name) makes a DIR in the current directory
 You MUST improve it to mkdir(char *pathname) for ANY pathname
****************************************************************/

int mkdir(char *name)
{
  NODE *p, *q;
  printf("mkdir: name=%s\n", name);

  //check for taken name
  if (!strcmp(name, "/") || !strcmp(name, ".") || !strcmp(name, "..")){
    printf("can't mkdir with %s\n", name);
    return -1;
  }
  
  dbname(name); //bname = basename, dname = dirname
  printf("dir_name %s and base_name %s\n", dname, bname);
  
  if (dname[0]=='/')
    start = root;
  else
    start = cwd;
  printf("starting from %s\n", start->name);
  
  //mkdir in cwd/root
  if(!strcmp(dname, ".") || !strcmp(dname, "/"))
  {
		printf("no path specified, creating in current dir\n");
		printf("check whether %s already exists\n", bname);
		p = search_child(start, bname);
		if (p){
		  printf("name %s already exists, mkdir FAILED\n", bname);
		  return -1;
		}
		p=start; //for insert
  }
  else //mkdir in dname
  {
		p=start; //p = cwd || root
		char *dir;
		dir = strtok(dname, "/");
		printf("searching for dir %s in parent dir %s\n", dir, p->name);
		while(dir)
		{
			p = search_child(p, dir);
			if(!p)
			{
			  printf("name %s not found mkdir FAILED\n", dir);
			  return -1; 
			}
			else if(p->type == 'F')
			{
			  printf("name %s is a FILE mkdir FAILED\n", dir);
			  return -1;
			}
			else
			  dir = strtok(0, "/"); //move to next token
		}
		printf("check if %s already exist\n", bname);
		q = search_child(p, bname);
		if(q){
		  printf("name %s already exist, mkdir FAILED\n", bname);
		  return -1;
		}
  }
  
  //insert node
  printf("--------------------------------------\n");
  printf("ready to mkdir %s\n", bname);
  q = (NODE *)malloc(sizeof(NODE));
  q->type = 'D';
  strcpy(q->name, bname);
  insert_child(p, q);
  printf("mkdir %s OK\n", bname);
  printf("--------------------------------------\n");
  
  return 0;
}

//---------PWD---------
int pwd_helper(NODE *p)
{
  if(p != root)
    pwd(p->parent);
  if(p->parent == root)
    printf("%s", p->name);
  else
    printf("/%s", p->name);
    
  return 0;
}

int pwd()
{
  pwd_helper(cwd);
}

//----------CD--------------
int cd(char *name)
{
  if(name[0]=='/')
    start = root;
  else
    start = cwd;
  NODE *p = start;
  
	char *dir;
	dir = strtok(name, "/");
	while(dir)
	{
		if(!strcmp(dir, ".."))
		{
			printf(".. found, moving up one directory\n");
			p = p->parent;
		}
		else
		{
			p = search_child(p, dir);
			if(p->type == 'F')
			{
				printf("name %s is a FILE cd FAILED\n", dir);
				return -1;
			}
			else if(!p)
			{
				printf("name %s not found cd FAILED\n", dir);
				return -1; 
			}
		}
		dir = strtok(0, "/"); //move to next token
	}

	printf("dir found, setting cwd to %s\n", pathname);
	cwd = p;
  
  return 0;
}

// This ls() list CWD. You MUST improve it to ls(char *pathname)
int ls(char *name)
{
  if (name[0]=='/')
    start = root;
  else
    start = cwd;
  NODE *p = start;  
  if(!name) //no name given
  {
    printf("no path, print cwd\n");
  }
  else
  {
    printf("check if pathname %s exist\n", name);
    char *dir;
    dir = strtok(name, "/");
    while(dir)
    {
      p = search_child(p, dir);
			if(!p)
			{
			  printf("name %s not found mkdir FAILED\n", dir);
			  return -1; 
			}
			else if(p->type == 'F')
			{
			  printf("name %s is a FILE mkdir FAILED\n", dir);
			  return -1;
			}
			else
			  dir = strtok(0, "/"); //move to next token
    }
  }
  
  p = p->child;
  printf("contents = ");
  while(p){
    printf("[%c %s] ", p->type, p->name);
    p = p->sibling;
  }
  printf("\n");
}

int quit()
{
  printf("Program exit\n");
  exit(0);
  // improve quit() to SAVE the current tree as a Linux file
  // for reload the file to reconstruct the original tree
}

//set root to "/"
int initialize()
{
    root = (NODE *)malloc(sizeof(NODE));
    strcpy(root->name, "/");
    root->parent = root;
    root->sibling = 0;
    root->child = 0;
    root->type = 'D';
    cwd = root;
    printf("Root initialized OK\n");
}


int main()
{

  int index;
  int ret;
  int (*fptr[ ])(char *)={(int (*)())mkdir,ls,quit,cd,pwd,0}; 
  //mkdir", "ls", "quit", "cd", "pwd", "rmdir", "rm", "creat", "save", "reload", 0

  initialize();

  printf("NOTE: commands = [mkdir|ls|quit|rmdir|creat|rm|pwd|save|reload]\n");

  while(1){
      printf("Enter command line : ");
      //get input
      fgets(line, 128, stdin);
      line[strlen(line)-1] = 0;

      command[0] = pathname[0] = 0;
      sscanf(line, "%s %s", command, pathname);
      printf("command=%s pathname=%s\n", command, pathname);
      
      if (command[0]==0) 
         continue;

      index = findCmd(command);
      if(index != -1)
        ret = fptr[index](pathname);
        
      
  }
}


