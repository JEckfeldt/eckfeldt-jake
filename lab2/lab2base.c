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

char *cmd[] = {"mkdir", "ls", "quit", "cd", "pwd", "rmdir", "creat", "rm", "save", "reload", "menu", 0};

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

int remove_child(NODE *parent, NODE *p)
{
  NODE *q;
  printf("remove NODE %s from NODE %s\n", q->name, parent->name);
  if(parent->child == p) //p is C(1)
  {
    parent->child = p->sibling;
    free(p);
  }
  else // p is middle of list OR end
  {
    //get node before p
    q = parent->child;
    while(q->sibling != p)
    {
      q = q->sibling;
    }
    q->sibling = p->sibling;
    free(p);
  }
  
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
//---------------MKDIR------------------------------------------------------------------------------------
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

//-------CREAT------------------------------------------------------------------------------------
int creat(char *name)
{
  NODE *p; //ptr
  NODE *q; //inserted
  dbname(name);
  printf("creat %s in %s\n", bname, dname);
  
  if(dname[0] == '/')
    start = root;
  else
    start = cwd;
  printf("starting from %s\n", start->name);
  
  //creat in cd
  if(!strcmp(dname, ".") || !strcmp(dname, "/"))
  {
    printf("creat in cd\n");
    printf("check if %s already exist\n", bname);
    p = search_child(start, bname);
		if (p){
		  printf("name %s already exists, creat FAILED\n", bname);
		  return -1;
		}
		p=start; //for insert
  }
  else
  {
    printf("Searching for path %s\n", dname);
    p=start; //p = cwd || root
		char *dir;
		dir = strtok(dname, "/");
		printf("searching for dir %s in parent dir %s\n", dir, p->name);
		while(dir)
		{
			p = search_child(p, dir);
			if(!p)
			{
			  printf("name %s not found CREAT FAILED\n", dir);
			  return -1; 
			}
			else
			  dir = strtok(0, "/"); //move to next token
		}
		printf("check if %s already exist\n", bname);
		q = search_child(p, bname);
		if(q){
		  printf("name %s already exist, creat FAILED\n", bname);
		  return -1;
		}
  }
  
  printf("inserting FILE %s in %s\n", bname, dname);
  printf("--------------------------------------\n");
  printf("ready to creat %s\n", bname);
  q = (NODE *)malloc(sizeof(NODE));
  q->type = 'F';
  strcpy(q->name, bname);
  insert_child(p, q);
  printf("creat %s OK\n", bname);
  printf("--------------------------------------\n");
  
  return 0;
}

//-------RM------------------------------------------------------------------------------------
int rm(char *name)
{
  NODE *p;
  NODE *parent;
  
  dbname(name);
  printf("remove FILE %s FROM %s\n", bname, dname);
  
  if(dname[0] == '/')
		start = root;
	else
	  start = cwd;
	printf("starting from %s\n", start->name);
	
	
	if(!strcmp(dname, ".") || !strcmp(dname, "/")) //rm from root
  {
    //rm dir
    printf("No path specified, removing from CD\n");
    parent = start;
    p = search_child(parent, bname);
    if(!p)
    {
      printf("FILE %s not exist rm FAILED\n", p->name);
      return -1;
    }
    if(p->type == 'D')
    {
      printf("%s is a DIR rm FAILED\n", p->name);
    }
    remove_child(parent, p);
    printf("Removed %s from %s\n", p->name, parent->name);
  }
  else
  {
    p = start;
    printf("Searching for path\n");
    char *dir = strtok(dname, "/");
    while(dir)
    {
      printf("Searching for %s\n", dir);
      p = search_child(p, dir);
			if(!p)
			{
			  printf("name %s not found rm FAILED\n", dir);
			  return -1; 
			}
			else
			  dir = strtok(0, "/"); //move to next token
    }
    
    printf("----Removing FILE----\n");
    parent = p;
    p = search_child(parent, bname);

    if(!p)
    {
      printf("name %s not exist rm FAILED\n", p->name);
      return -1;
    }
    if(p->type == 'D')
    {
      printf("%s is a DIR rm FAILED\n", p->name);
    }
    remove_child(parent, p);
    printf("Removed %s from %s\n", bname, parent->name);
    
  }
	
}

//-----RMDIR-------------------------------------------------------------------------------------
int rmdir(char *name)
{
  NODE *p; //to be deleted
  NODE *parent;
  
  dbname(name);
  printf("dir_name %s base_name %s\n", dname, bname);
  
  if (dname[0]=='/')
    start = root;
  else
    start = cwd;
  printf("Starting from %s\n", start->name);
  
  if(!strcmp(dname, ".") || !strcmp(dname, "/")) //rm from root
  {
    //rm dir
    printf("No path specified, removing from CD\n");
    parent = start;
    p = search_child(parent, bname);
    if(p->child)
    {
      printf("name %s is not EMPTY rmdir FAILED\n", p->name);
      return -1;
    }
    if(!p)
    {
      printf("name %s not exist rmdir FAILED\n", p->name);
      return -1;
    }
    remove_child(parent, p);
    printf("Removed %s from %s\n", p->name, parent->name);
  }
  else
  {
  	p = start;
    printf("Searching for path\n");
    char *dir = strtok(dname, "/");
    while(dir)
    {
      printf("Searching for %s\n", dir);
      p = search_child(p, dir);
			if(!p)
			{
			  printf("name %s not found rmdir FAILED\n", dir);
			  return -1; 
			}
			else if(p->type == 'F')
			{
			  printf("name %s is a FILE rmdir FAILED\n", dir);
			  return -1;
			}
			else
			  dir = strtok(0, "/"); //move to next token
    }
    
    printf("----Removing dir----\n");
    parent = p;
    p = search_child(parent, bname);
    if(p->child)
    {
      printf("name %s is not EMPTY rmdir FAILED\n", p->name);
      return -1;
    }
    if(!p)
    {
      printf("name %s not exist rmdir FAILED\n", p->name);
      return -1;
    }
    remove_child(parent, p);
    printf("Removed %s from %s\n", p->name, parent->name);
  }
  
  
  return 0;
  
}

//---------PWD------------------------------------------------------------------------------------
int pwd(NODE *p)
{
  if(p != root)
    pwd(p->parent);
  if(p->parent == root)
    printf("%s", p->name);
  else
    printf("/%s", p->name);
    
  return 0;
}

//----------CD------------------------------------------------------------------------------------
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

//--------RELOAD----------------------------------------------------------------------------------
int reload()
{
  FILE *fp = fopen("save_file", "r");
  char line[128];
  while(fgets(line, sizeof line, fp))
  {
    char *type, *path;
    
    type = strtok(line, " ");
    printf("%s\n", type);
    
    path = strtok(0, "\n");
    printf("%s\n", path);
    
    if(*type == 'D')
      mkdir(path);
    else
      creat(path);
  }
  
}

//-------SAVE HELPERS----------------------------------------------------------------------------
int fpwd(NODE *p, FILE *file)
{
  if(p != root)
    fpwd(p->parent, file);
  if(p->parent == root)
    fprintf(file, "%s", p->name);
  else
    fprintf(file, "/%s", p->name);
    
  return 0;
}

int print_pre_order(NODE *node, FILE* file)
{
  if(!node)
    return 0;
  else
  {
    //print to file
    fprintf(file, "%c ", node->type);
    fpwd(node, file);
    fprintf(file, "\n");
    
    //recursive call
    print_pre_order(node->child, file);
    print_pre_order(node->sibling, file);
  }
}

//--------SAVE----------------------------------------------------------------------------------
int save()
{
  FILE *fp = fopen("save_file", "w+");
  print_pre_order(root, fp);
  fclose(fp);
}

//--------quit----------------------------------------------------------------------------------
int quit()
{
  printf("Saving Tree\n");
  save();
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

int menu()
{ 
 printf("NOTE: COMMANDS = [mkdir|ls|quit|rmdir|pwd|creat|rm|save|reload|menu]\n");
 return 0;
}

int main()
{
  initialize();

  printf("NOTE: commands = [mkdir|ls|quit|rmdir|pwd|creat|rm|save|reload|menu]\n");

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

      switch(findCmd(command)){
        case 0: mkdir(pathname);  break;
        case 1: ls(pathname);  break;
        case 2: quit(pathname);  break;
        case 3: cd(pathname);  break;
        case 4: pwd(cwd); printf("\n");  break;
        case 5: rmdir(pathname);  break;
        case 6: creat(pathname);  break;
        case 7: rm(pathname);  break;
        case 8: save();  break;
        case 9: reload();  break;
        case 10: menu();  break;
      }
        
      
  }
  printf("program exit\n");
  return 0;
}


