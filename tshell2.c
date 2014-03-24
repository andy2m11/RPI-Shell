#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include  <signal.h>
#include  <sys/types.h>
#include  <fcntl.h>

#define MAX_LINE 1024
/*
cmd > file Redirect the standard output (stdout) of cmd to a file
cmd 1> file Same as cmd > file. 1 is the default file descriptor (fd) for stdout.
cmd 2> file Redirect the standard error (stderr) of cmd to a file. 2 is the default fd for stderr.
cmd >> file Append stdout of cmd to a file.
cmd 2>> file Append stderr of cmd to a file.
cmd &> file Redirect stdout and stderr of cmd to a file
cmd < file Redirect the contents of the file to the standard input (stdin) of cmd.
cmd1 | cmd2 Redirect stdout of cmd1 to stdin of cmd2.
*/
struct arguments{
	int rc;		//redirect stdout to file	
	int rc1;	//""
	int rc2;	//redirect stderr to file
	int rcrc;	//append stdout to file
	int rcrc2;	//append stderr to file
	int rcand;	//redirect stdout and stderr to file
	int lc;		//redirect contents of file to stdin
	int pipe;	//redirect stdout of cm1 to stdin of cmd2
	int sel;	
	char * redir_s; //special redirection/pipe symbol 
	int c;	
} ar;

int isRedir(char* arg)
{
	int act = 0;
	switch(arg[0])
	{
		case '>':
			if(arg[1] == '>')
			{
				ar.sel = 1;
				act = 1;
			}
			else
			{
				ar.sel = 3;
				act = 3;
			}
			
			break;	
		case '1':
			ar.sel = 1;
			act = 1;
			break;
		case '2':
			if(arg[1] == '>' && arg[2] == '>')
			{
				ar.sel = 4;
				act = 4;
			}
			else
			{
				ar.sel = 2;
				act = 2;
			}
			
			break;
		case '&':
			ar.sel = 5;
			act = 5;
			break;
		case '<':
			ar.sel = 6;
			act = 6;
			break;
		case '|':
			ar.sel = 7;
			act = 7;
			break;
		default:
			ar.sel = 0;
			act = 0;
			break;
			
	}

return act;
}

int doarg(char **argv, char **argv2)
{
       int fd;
       int fd2[2];
       pid_t pid, pid2;
        pid = fork();
        if (pid < 0){
            printf("Fork Failed\n");
        }
        else if (pid == 0) // child process 
        { 
        	switch(ar.sel)
        	{
        		case 0://
        			setpgid(0,0);
	 			if(execvp(argv[0], argv) == -1)
				{
				   fprintf(stderr, "%s is not a valid command\n",argv[0]);
	 			   exit(EXIT_FAILURE);
				}
				break;
      			case 1://
             			fd = open(argv2[0], O_CREAT | O_WRONLY , S_IRUSR | S_IWUSR | S_IXUSR);
              			dup2(fd,1); 
	 			if(execvp(argv[0], argv) == -1)
				{
				   fprintf(stderr, "%s is not a valid command\n",argv[0]);
	 			   exit(EXIT_FAILURE);
				}                            
				break;   		
        		case 2://
             		        fd = open(argv2[0], O_CREAT | O_WRONLY , S_IRUSR | S_IWUSR | S_IXUSR);
              			dup2(fd,2);        	
  	 			if(execvp(argv[0], argv) == -1)
				{
				   fprintf(stderr, "%s is not a valid command\n",argv[0]);
	 			   exit(EXIT_FAILURE);
	 			}
	 			break;               	
        		case 3://
                		fd = open(argv2[0], O_CREAT | O_WRONLY | O_APPEND , S_IRUSR | S_IWUSR | S_IXUSR);
               			dup2(fd,1);        		
                		execvp(argv[0], argv); 
                		break;
        		case 4://
        			fd = open(argv2[0], O_CREAT | O_WRONLY | O_APPEND , S_IRUSR | S_IWUSR | S_IXUSR);
                		dup2(fd,2);
               			execvp(argv[0], argv);
               			break;
        		case 5://
				fd = open(argv2[0], O_CREAT | O_WRONLY , S_IRUSR | S_IWUSR | S_IXUSR);
				dup2(fd,1);
				dup2(fd,2);
				execvp(argv[0], argv);       
				break;		
        		case 6://
				fd = open(argv2[0], O_CREAT | O_RDONLY , S_IRUSR | S_IWUSR | S_IXUSR);
				dup2(fd,0);
				execvp(argv[0], argv); 
				break;    		
        		case 7://
				pipe(fd2);
				pid2 = fork();
				if (pid2 < 0){
				    printf("Fork2 Failed\n");
				}
				else if (pid2 == 0){
				    dup2(fd2[1],1);
				    close(fd2[0]);
				    execvp(argv[0], argv);
				}
				else 
				{
				    dup2(fd2[0],0);
				    close(fd2[1]);
				    execvp(argv2[0], argv2);   
				}   
				break;  		
        		//default:
        		
		}
	}
}


int count_args(char* line)
{
   int words=0, in_word=0;  

   while(*line)
   {
      if(isspace(*line))
      {
	 in_word = 0;
      }
      else
      {
	 if(in_word == 0)
	 {
	    words++;
	    in_word = 1;
	 }
      }
      line++;
   }
   return words;
}


char **build_argv(char* line)
{
   int argc = count_args(line);
   ar.c = argc;
   int i, act;
   char *new;
   char **argv;

   if(argc ==0)
   {
      return NULL;
   }

   argv = malloc(sizeof(char*)*(argc+1));
   if(!argv)
   {
      fprintf(stderr, "malloc() failure -- out of memory");
      exit(EXIT_FAILURE);
   }

   for(i=0; i<argc; i++)
   {
      while(isspace(*line))
      {
	 line++;
      }
      for(new=line; *new && !isspace(*new); new++);
      /* Empty body */
      *new = '\0';
      argv[i] = malloc(strlen(line)+1);
      if(!argv)
      {
         fprintf(stderr, "malloc() failure -- out of memory");
         exit(EXIT_FAILURE);
      }
      act = isRedir(line);
      if(ar.sel != act)
      {
      	 fprintf(stderr, "Act != ar.sel");
      }
      if(ar.sel == 0 )
      {
      	strcpy(argv[i], line);
      }

      line = new+1;
   }
   argv[i] = NULL;
   return argv;
}

void print_argv(char **argv)
{
   int i;
   printf("Command: %s\n", argv[0]);
   printf("%s","Arguments:\n");
   for(i=1; argv[i]; i++)
   {
      printf("argv[%d]: ", i);
      printf("%s\n", argv[i]);
   }
}

int main(void)
{
   pid_t  child_pid;
   char  line[MAX_LINE];
   char  *line_res;
   char **argv;
   
   while(1)
   {
//      ar.pval = "~/Git/RPI-SHELL/";
//      myftw(ar.pval,myfunc);
      fprintf(stderr,"%s","$ ");
      line_res = fgets(line, MAX_LINE, stdin);
      if(!line_res)
      {	 break;  }
 //     gopt(ar.c,argv);      
 
      argv = build_argv(line);    
      print_argv(argv);
      
//      doarg(argv, argv);
      child_pid = fork();
      if(child_pid == -1)
      {
	 fprintf(stderr, "%s\n", "fork() failure");
	 exit(EXIT_FAILURE);
      }
      if(child_pid == 0) 
      {
	 setpgid(0,0);
	 if(execvp(argv[0], argv) == -1)
	 {
	    fprintf(stderr, "%s is not a valid command\n",argv[0]);
	    exit(EXIT_FAILURE);
	 }
      }
      else
      {
	 setpgid(child_pid, child_pid);
	 if(wait(NULL) == -1)
	 {
	    fprintf(stderr, "%s\n", "wait() failure");
	    exit(EXIT_FAILURE);
	 }
	 free(argv);
      }  
      
   }
   
   return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------





