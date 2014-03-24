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
	char *cmd1;
	char *cmd2;
	int rplace;
} ar;

int isRedir(char* arg)
{
	int act = 0;
	switch(arg[0])
	{
		case '>':
			if(arg[1] == '>')
			{
				fprintf(stderr, "case: %c\n", arg[0]);
				ar.sel = 3;
				act = 3;
			}
			else
			{
				fprintf(stderr, "case: %c\n", arg[0]);			
				ar.sel = 1;
				act = 1;
			}
			
			break;	
		case '1':
			fprintf(stderr, "case: %c\n", arg[0]);		
			ar.sel = 1;
			act = 1;
			break;
		case '2':
			fprintf(stderr, "case: %c\n", arg[0]);
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
			fprintf(stderr, "case: %c\n", arg[0]);		
			ar.sel = 5;
			act = 5;
			break;
		case '<':
			fprintf(stderr, "case: %c\n", arg[0]);		
			ar.sel = 6;
			act = 6;
			break;
		case '|':
			fprintf(stderr, "case: %c\n", arg[0]);		
			ar.sel = 7;
			act = 7;
			break;
		default:
			fprintf(stderr, "Default Entered\n");		
			ar.sel = 0;
			act = 0;
			break;
	}
	


return act;
}

int doarg(char **argv, int act)
{
       int fd;
       int fd2[2];
       pid_t pid, pid2;
       char ** argv2;
       fprintf(stderr, "argv[0]:%s, placement:%d, aact:%d, argv[3]:%s \n", argv[0], ar.rplace, act, argv[ar.rplace+1]);
       
       
       pid = fork();
  
        if (pid < 0){
            printf("Fork Failed\n");
        }
        else if (pid == 0) // child process 
        { 
        	switch(act)
        	{
        		case 0:// no redirection or pipes
        			setpgid(0,0);          			
	 			execvp(argv[0], argv);
				break;
      			case 1:// > or a 1>
	      			setpgid(0,0);
       fprintf(stderr, "argv[0]:%s, a[1]:%s, argv[2]:%s \n", argv[0], argv[1], argv[ar.rplace+1]); 	      			
             			fd = open(argv[ar.rplace+1], O_CREAT | O_WRONLY , S_IRUSR | S_IWUSR | S_IXUSR);
              			dup2(fd,1); 
                 		argv[1] = '\0';
              			argv[2] = '\0';             			
	 			execvp(argv[0], argv);
                       
				break;   		
        		case 2://2>
             		        fd = open(argv2[0], O_CREAT | O_WRONLY , S_IRUSR | S_IWUSR | S_IXUSR);
              			dup2(fd,2);        	
  	 			if(execvp(argv[0], argv) == -1)
				{
				   fprintf(stderr, "%s is not a valid command\n",argv[0]);
	 			   exit(EXIT_FAILURE);
	 			}
	 			break;               	
        		case 3://>>
                		fd = open(argv2[0], O_CREAT | O_WRONLY | O_APPEND , S_IRUSR | S_IWUSR | S_IXUSR);
               			dup2(fd,1);        		
                		execvp(argv[0], argv); 
                		break;
        		case 4://2>>
        			fd = open(argv2[0], O_CREAT | O_WRONLY | O_APPEND , S_IRUSR | S_IWUSR | S_IXUSR);
                		dup2(fd,2);
               			execvp(argv[0], argv);
               			break;
        		case 5://&
				fd = open(argv2[0], O_CREAT | O_WRONLY , S_IRUSR | S_IWUSR | S_IXUSR);
				dup2(fd,1);
				dup2(fd,2);
				execvp(argv[0], argv);       
				break;		
        		case 6:// <
				fd = open(argv2[0], O_CREAT | O_RDONLY , S_IRUSR | S_IWUSR | S_IXUSR);
				dup2(fd,0);
				execvp(argv[0], argv); 
				break;    		
        		case 7:// |
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
	else if (ar.sel != 5){ /* parent process */
	printf("running parent\n");
            pid_t childpid;
            int status;
            childpid = wait(&status); //wait the child process to finish
            if (childpid < 0){
                printf("wait error");
                exit(1);
            }
        }
     return 0;   
}


int count_args(char* line)
{
   int words=0, in_word=0;  
   int redirect = 0;
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
//	    redirect = isRedir(line);
//	    if(redirect == 0)
//	    {
	      words++;
	      in_word = 1;
//	    }
	 }
      }
      line++;
   }
   return words;
}


char **build_argv(char* line, int *act)
{
   int argc = count_args(line);
   ar.c = argc;
   int i, fdone; fdone = 0; 
   char *new;
   char **argv;
   int arg_count = 0;
   int final = 0;
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
 	      *act = isRedir(line);
 	      if(ar.sel != act)
	      {    fprintf(stderr, "Act != ar.sel\n");	      }

	      while(isspace(*line))
	      {
		 line++;
	      }
	      for(new=line; *new && !isspace(*new)/* && (act == 0)*/; new++);
	      /* Empty body */
	      *new = '\0';
	      argv[i] = malloc(strlen(line)+1);
	      if(!argv)
	      {
		 fprintf(stderr, "malloc() failure -- out of memory\n");
		 exit(EXIT_FAILURE);
	      }
	      fprintf(stderr, "argc %d\n", argc);
	      
	      if(ar.sel == 0 )
	      {
	      	strcpy(argv[i], line);
	      	arg_count++;
	      }
	      else
	      {
	      	ar.rplace = i;
	        fprintf(stderr, "sel is:%d   Line is:%s\n",ar.sel,line);
		final = ar.sel;
	      }
	      line = new+1;
   }
   argv[i] = NULL;
   ar.c = arg_count;
   *act = final;
   return argv;
}


int main(void)
{
   pid_t  child_pid;
//   char  line[MAX_LINE];
   char  *line_res;
   char **argv;
   int act = 0;
   char* line;
   while(1)
   {
//      ar.pval = "~/Git/RPI-SHELL/";
//      myftw(ar.pval,myfunc);
      fprintf(stderr,"%s","$ ");
      line_res = fgets(line, MAX_LINE, stdin);
      if(!line_res)
      {	 break;  }

 /////////////////////////////////////////////////////////////////////
   int argc = 0;
  	//-------------------
  	
   int words=0, in_word=0;  
   int redirect = 0;
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
//	    redirect = isRedir(line);
//	    if(redirect == 0)
//	    {
	      words++;
	      in_word = 1;
//	    }
	 }
      }
      line++;
   }
   
   //-----------------------------
   ar.c = argc;
   int i, fdone; fdone = 0; 
   char *new;
   int arg_count = 0;
   int final = 0;


   argv = malloc(sizeof(char*)*(argc+1));
   if(!argv)
   {
      fprintf(stderr, "malloc() failure -- out of memory");
      exit(EXIT_FAILURE);
   }

   for(i=0; i<argc; i++)
   {
 	      act = isRedir(line);
 	      if(ar.sel != act)
	      {    fprintf(stderr, "Act != ar.sel\n");	      }

	      while(isspace(*line))
	      {
		 line++;
	      }
	      for(new=line; *new && !isspace(*new)/* && (act == 0)*/; new++);
	      /* Empty body */
	      *new = '\0';
	      argv[i] = malloc(strlen(line)+1);
	      if(!argv)
	      {
		 fprintf(stderr, "malloc() failure -- out of memory\n");
		 exit(EXIT_FAILURE);
	      }
	      fprintf(stderr, "argc %d\n", argc);
	      
	      if(ar.sel == 0 )
	      {
	      	strcpy(argv[i], line);
	      	arg_count++;
	      }
	      else
	      {
	      	ar.rplace = i;
	        fprintf(stderr, "sel is:%d   Line is:%s\n",ar.sel,line);
		final = ar.sel;
	      }
	      line = new+1;
   }
   argv[i] = NULL;
   ar.c = arg_count;
   act = final;

      //////////////////////////////////////////////////////////////

   printf("Command: %s\n", argv[0]);
   printf("%s","Arguments:\n");
   for(i=1; argv[i]; i++)
   {
      printf("argv[%d]: ", i);
      printf("%s\n", argv[i]);
   }
      ///////////////////////////////////////////////////////////////////
      doarg(argv, act);
 
   }

   return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------





