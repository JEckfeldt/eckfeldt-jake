#include <stdio.h>
#include <stdlib.h> // calloc(), malloc(), exit()
#include <envz.h>   // envz_get()
#include <string.h> // strtok()
#include <time.h>   // time(), localtime()
#include <fcntl.h>  // O_RDONLY, O_WRONLY, O_APPEND, O_CREAT
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

//#define clear() printf("\033[H\033[J")

// state is for identifying what type of command the user inputs.
typedef enum
{
	BASIC,
	REDIRECT,
	PIPE
} state;

int numDirs = 0; // number of bin directory
char *home; // home directory
char *dir[64]; // holds all the paths for bins
char line[128]; // for getting user input line
char command[16], pathname[64]; // for command and pathname strings
char *name[64]; // command tokenized into arguments
int nnames = 0; // number of tokens in name[]
char *myargv[64]; // holds all of the command tokens as well as redirect and pipe symbols for each user input line.
// char *head[64]; // holds the head command for piping.
// char *tail[64]; // holds the tail command for piping.

//int commandType(char *cmd, char *env[]);

// initializes char * arrays so that strcpy doesn't segfault.
void initialize()
{
	for (int i = 0; i < 64; i++)
	{
		dir[i] = (char*)malloc(sizeof(" "));
		//name[i] = (char*)malloc(sizeof(" "));
		//myargv[i] = (char*)malloc(sizeof(" "));
	}
}

// breaks up the paths for the bin directories. saves the paths in dir[].
void findHomePath()
{
	char * token;
	char temp[320];
	int j = 0;

	home = getenv("HOME");
	strcpy(temp, getenv("PATH"));

	token = strtok(temp, ":");
	while (token)
	{
		strcpy(dir[j], token);
		//dir[j] = token;
		printf("%s\n", dir[j]);
		token = strtok(0, ":");
		j++;
	}
	numDirs = j;
}

// breaks up the user command into arguments and places them in name[]
void tokenize(char *cmd)
{
	char * token; 
	int j = 0;

	token = strtok(cmd, " ");
	while (token)
	{
		name[j] = token;
		token = strtok (0, " ");
		j++;
	}
	nnames = j;
}


//this is only used in piping. really sloppy code but I am running out of time so if it works then it's good enough.
int redirectCmd(char *cmd[], char *env[], int num)
{
	int l;
	char temp1[100];
	state type = BASIC;
	char direction[4];
	char fileName[256];
	char *command[64];

	for (int j = 0; j < num; j++)
	{
		fprintf(stderr, "cmd[%d]: %s\n", j, cmd[j]);
		if (strcmp(cmd[j], "<") == 0 || strcmp(cmd[j], ">") == 0 || strcmp(cmd[j], ">>") == 0)
		{ 
			type = REDIRECT;
			strcpy(direction, cmd[j]);
			strcpy(fileName, cmd[j+1]);
			fprintf(stderr, "File 1: %s\nDirection: %s\nFile 2: %s\n", cmd[j-1], direction, fileName);
			command[j] = '\0';
			break;
		}
			command[j] = (char*)malloc(sizeof(" "));
			strcpy(command[j], cmd[j]);
	}

	if (type != REDIRECT) //execute a basic command instead
	{
		fprintf(stderr, "No redirection required. Returning -1.\n");
		//sleep(1);
		return -1;
	}

	//fprintf(stderr, "command[2]: %d\n", command[2]);

	//redirect to output file or input. < takes input from infile. > sends outputs to outfile. >> append output to outfile
	if (strcmp(direction, "<") == 0)
	{
		close(0);
		open(fileName, O_RDONLY);
		fprintf(stderr, "%d OPENING %s in READ MODE\n", getpid(), fileName);
	}
	else if (strcmp(direction, ">") == 0)
	{
		close(1);
		open(fileName, O_WRONLY | O_CREAT, 0644);
		fprintf(stderr, "%d OPENING %s in WRITE MODE\n", getpid(), fileName);
	}
	else if (strcmp(direction, ">>") == 0)
	{
		close(1);
		open(fileName, O_WRONLY | O_APPEND, 0644);
		fprintf(stderr, "%d OPENING %s in APPEND MODE\n", getpid(), fileName);
	}
	else
	{
		fprintf(stderr, "Redirection error.\n");
		exit(-1);
	}

	//return 1;

	for (int i = 0; i < numDirs; i++)
	{
		//if (dir[i]+"/"+command ) // check if the directory has the command. JUST PUT THIS IN EXECV. DON'T NEED AN IF PROBABLY
		//execv(path, myargv) where myargv is the command and all the arguments as indexes in an array
		strcpy(temp1, dir[i]);
		strcat(temp1, "/");
		strcat(temp1, command[0]);
		fprintf(stderr, "Searching:  %s\n", temp1);
		if (-1 != execve(temp1, command, env))
		{
			//fprintf(stderr, "SUCCESS\n");
		}
	}

	return 1;
}

// TEST WITH THIS: cat filename | grep test >> newtextfile.txt
//executes the users pipe command
int executePipe(char *env[])
{
	char temp1[100];
	char temp2[100];
	int pid, status;
	int pd[2];
	int l=0, n=0, check1 = 0, check2 = 0;
	char *head[128]; // holds the head command for piping.
	char *tail[128]; // holds the tail command for piping.

        pipe(pd);        // creates a PIPE; pd[0] for READ  from the pipe, 
                         //                 pd[1] for WRITE to   the pipe.
		fprintf(stderr, "head: ");
		for (int k = 0; k < nnames; k++)
		{
			if (strcmp(name[k], "|") == 0)
			{
				l = k+1;
				//fprintf(stderr, "name[k]: %s\nname[l]: %s\n", name[k], name[l]);
				head[l-1] = '\0';
				//printf("l: %d\n", l);
				break;
			}
			head[k] = (char*)malloc(sizeof(" "));
			head[k] = name[k];
			fprintf(stderr, "%s ", head[k]);
		}
		fprintf(stderr, "\ntail: ");
		for (int m = l; m < nnames; m++)
		{
			tail[n] = (char*)malloc(sizeof(" "));
			tail[n] = name[m];
			fprintf(stderr, "%s ", tail[n]);
			n++;
		}
		tail[n] = '\0';
		fprintf(stderr, "\n");
		//printf("n: %d\n", n);
		//printf("head[l-1]: %s\ntail[n]: %s\n", head[l-1], tail[n]);

		//printf("head[0]: %s\nhead[1]: %s\n", head[0], head[1]);
		//printf("tail[0]: %s\ntail[1]: %s\n", tail[0], tail[1]);

        pid = fork();    // fork a child process
                         // child also has the same pd[0] and pd[1]

        if (pid) {        // parent as pipe pipe WRITER
			//pid = wait(&status);
        	close(pd[0]); // WRITER MUST close pd[0]

            close(1);     // close 1
        	dup(pd[1]);   // replace 1 with pd[1]
        	close(pd[1]); // close pd[1] since it has replaced 1
			check1 = redirectCmd(head, env, l-1); // opens file if redirection is needed.
			if (check1 == -1) // file is closed.
			{
				//sleep(1);
				for (int i = 0; i < numDirs; i++)
				{
					//exec(head);   // change image to cmd1
					strcpy(temp1, dir[i]);
					strcat(temp1, "/");
					strcat(temp1, head[0]);
					//fprintf(stderr, "head[0]: %s\nhead[1]: %s\n", head[0], head[1]);
					fprintf(stderr, "Searching:  %s\n", temp1);
					execve(temp1, head, env);
				}
				sleep(1);
			}
			// else //file is open. do redirection before second half of pipe. pass output of redirection to pipe somehow?
			// {
			// 	// execute command normally but make sure to not pass anything after >, <, >> in tail or head.
			// }
			//commandType(head, env);
			//exit(0);
        }
        else {            // child as pipe pipe READER
            close(pd[1]); // READER MUST close pd[1]

            close(0);  
            dup(pd[0]);   // replace 0 with pd[0]
            close(pd[0]); // close pd[0] since it has replaced 0
			check2 = redirectCmd(tail, env, n); //opens file if redirection is needed.
			if (check2 == -1) // file is closed
			{
				for (int i = 0; i < numDirs; i++)
				{
					//exec(tail);   // change image to cmd2
					strcpy(temp2, dir[i]);
					strcat(temp2, "/");
					strcat(temp2, tail[0]);
					//fprintf(stderr, "tail[0]: %s\ntail[1]: %s\n", tail[0], tail[1]);
					fprintf(stderr, "Searching:  %s\n", temp2);
					execve(temp2, tail, env);
				}
			}
			// else // file is open. do redirection after second half of pipe. pass output of pipe to file somehow?
			// {
			// 	// execute command normally but make sure to not pass anything after >, <, >> in tail or head.
			// }
			//commandType(tail, env);
			//exit(0);
        }
}

//checks the type of command and runs it appropriately (basic, redirect, pipe).
int commandType(char *cmd, char *env[])
{
	int l;
	char temp1[100];
	state type = BASIC;
	char direction[4];
	char fileName[256];

	for (int z = 0; z < nnames; z++)
	{
		if (strcmp(name[z], "|") == 0) 
		{ 
			type = PIPE;
			break;
		}
	}

	if (type != PIPE)
	{
		for (int j = 0; j < nnames; j++)
		{
			if (strcmp(name[j], "<") == 0 || strcmp(name[j], ">") == 0 || strcmp(name[j], ">>") == 0)
			{ 
				type = REDIRECT;
				strcpy(direction, name[j]);
				strcpy(fileName, name[j+1]);
				fprintf(stderr, "File 1: %s\nDirection: %s\nFile 2: %s\n", name[j-1], direction, fileName);
				break;
			}
			// else if (strcmp(name[j], "|") == 0) 
			// { 
			// 	type = PIPE;
			// 	break;
			// }
			else
			{ 
				type = BASIC;
			}

			myargv[j] = (char*)malloc(sizeof(" "));
			strcpy(myargv[j], name[j]);
		}
	}

	// for (int x = 0; x < nnames; x++)
	// {
	// 	fprintf(stderr, "myargv[%d]: %s\n", x, myargv[x]);
	// }

	switch (type)
	{
		//MYARGV IS ALL OF NAME UNTIL IT REACHES ONE OF THE REDIRECT SYMBOlS FIRST.
		case BASIC:
				//printf("in child\n");
				//printf("dir[0] = %s\n", dir[0]);
				for (int i = 0; i < numDirs; i++)
				{
					//if (dir[i]+"/"+cmd ) // check if the directory has the command. JUST PUT THIS IN EXECV. DON'T NEED AN IF PROBABLY
					//execv(path, myargv) where myargv is the command and all the arguments as indexes in an array
					strcpy(temp1, dir[i]);
					strcat(temp1, "/");
					strcat(temp1, myargv[0]);
					fprintf(stderr, "Searching:  %s\n", temp1);
					execve(temp1, myargv, env);
				}
			break;

		case REDIRECT:
				//redirect to output file or input. < takes input from infile. > sends outputs to outfile. >> append output to outfile
				if (strcmp(direction, "<") == 0)
				{
					close(0);
					open(fileName, O_RDONLY);
					fprintf(stderr, "%d OPENING %s in READ MODE\n", getpid(), fileName);
				}
				else if (strcmp(direction, ">") == 0)
				{
					close(1);
					open(fileName, O_WRONLY | O_CREAT, 0644);
					fprintf(stderr, "%d OPENING %s in WRITE MODE\n", getpid(), fileName);
				}
				else if (strcmp(direction, ">>") == 0)
				{
					close(1);
					open(fileName, O_WRONLY | O_APPEND, 0644);
					fprintf(stderr, "%d OPENING %s in APPEND MODE\n", getpid(), fileName);
				}
				else
				{
					fprintf(stderr, "Redirection error.\n");
					exit(-1);
				}

				for (int i = 0; i < numDirs; i++)
				{
					//if (dir[i]+"/"+cmd ) // check if the directory has the command. JUST PUT THIS IN EXECV. DON'T NEED AN IF PROBABLY
					//execv(path, myargv) where myargv is the command and all the arguments as indexes in an array
					strcpy(temp1, dir[i]);
					strcat(temp1, "/");
					strcat(temp1, myargv[0]);
					fprintf(stderr, "Searching:  %s\n", temp1);
					execve(temp1, myargv, env);
				}
			break;

		case PIPE:
				//create a fork and execute pipe commands
				// fprintf(stderr, "head: ");
				// for (int k = 0; k < nnames; k++)
				// {
				// 	if (strcmp(name[k], "|") == 0)
				// 	{
				// 		l = k+1;
				// 		//fprintf(stderr, "name[k]: %s\nname[l]: %s\n", name[k], name[l]);
				// 		break;
				// 	}
				// 	head[k] = (char*)malloc(sizeof(" "));
				// 	strcpy(head[k], name[k]);
				// 	fprintf(stderr, "%s ", head[k]);
				// }
				// fprintf(stderr, "\ntail: ");
				// while (l < nnames)
				// {
				// 	tail[l] = (char*)malloc(sizeof(" "));
				// 	strcpy(tail[l], name[l]);
				// 	fprintf(stderr, "%s ", tail[l]);
				// 	l++;
				// }
				// fprintf(stderr, "\n");
				executePipe(env);
			break;
	}
}

//reinitialize myargv and the head and tail variables for pipes to be empty.
int delMyargv()
{
	for (int i = 0; i < 63; i++)
	{
		myargv[i] = NULL;
		// free(head[i]);
		// free(tail[i]);
	}
}

//execute bash commands.
int executeBash(char *env[])
{
	char buf[256];
	char c;
	char *cmd = "/usr/bin/bash\0";
	FILE *fp = fopen(name[1], "r");

	for (int i = 0; i < 256; i++)
	{
		c = fgetc(fp);
		buf[i] = c;
		//fprintf(stderr, "buf[%d]: %c\n", i, buf[i]);
	}
	fclose(fp);
	if (strncmp(buf, "#!", 2) == 0)
	{
		//fprintf(stderr, "bash is possible!\n");
		execve(cmd, name, env);
	}
}

int executeCommand(char *cmd, char *env[])
{
	int pid = fork(), status;
	if (pid)
	{
		//this is the parent process
        fprintf(stderr, "PARENT %d WAITS FOR CHILD %d TO DIE\n", getpid(),pid);
		pid = wait(&status);
		//print childs pid and exit status;
		sleep(1);
		fprintf(stderr, "CHILD %d DIED WITH STATUS CODE %04x\n", pid, status);
		/*************************************************DELETE ALL OF MYARGV************************************************************/
		delMyargv();
		//repeat getting commands by exiting the function.
	}
	else if (pid == 0)
	{
		//this is a child process
		if (strcmp(name[0], "bash") == 0)
		{
			executeBash(env);
		}
		else
		{
			commandType(cmd, env);
			sleep(1);
		}
        fprintf(stderr, "child %d dies by exit(0)\n", getpid());
		exit(0);
	}
	else
	{
		//error
		fprintf(stderr, "Fork failed\n");
		exit(-1);
	}
}

int main(int argc, char *argv[], char *env[])
{
	initialize();
	fprintf(stderr, "HOME PATH:\n");
	findHomePath();
	// sleep(3);
	// clear();
	fprintf(stderr, "\nENTER COMMAND:\n");
	
	while(1)
	{
		//get user input
		printf("SHELL: ");
		fgets(line, 128, stdin); 
		line[strlen(line)-1] = 0; 
		

		tokenize(line);

		if (strcmp(name[0], "cd") == 0)
		{
			//chdir(arg1) OR chdir($HOME) if no arg1
			if (name[1] != NULL)
			{
				chdir(name[1]);
                fprintf(stderr, "%d cd to %s\n", getpid(), name[1]);
			}
			else
			{
				chdir(home); // find $HOME from *env[].
                fprintf(stderr, "%d cd to HOME\n", getpid());
			}
		}
		else if (strcmp(name[0], "exit") == 0)
		{
			fprintf(stderr, "%d exit\n", getpid());
			exit(1);
		}
		else
		{
			executeCommand(line, env);
		}
		//reinitialize name to NULL
		for (int j = 0; j < nnames; j++)
		{
			name[j] = NULL;
		}
		nnames = 0;
	}
	return 0;
}

