#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include  <signal.h>
#include  <fcntl.h>
#include  <linux/stat.h>

#define MAX_LINE 1024
/*
struct Arguments{
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
}ar;
*/
//This is where the action for redirection and pipes are done
int isRedir(char* arg)
{
	int act = 0;
	switch(arg[0])
	{
		case '>':
			if(arg[1] == '>')
			{
//				fprintf(stderr, "case: %c\n", arg[0]);
//				ar.sel = 3;
				act = 3;
			}
			else
			{
			fprintf(stderr, "case: %c\n", arg[0]);			
//				ar.sel = 1;
				act = 1;
			}
			
			break;	
		case '1':
			//fprintf(stderr, "case: %c\n", arg[0]);		
//			ar.sel = 1;
			act = 1;
			break;
		case '2':
			//fprintf(stderr, "case: %c\n", arg[0]);
			if(arg[1] == '>' && arg[2] == '>')
			{
//				ar.sel = 4;
				act = 4;
			}
			else
			{
//				ar.sel = 2;
				act = 2;
			}
			
			break;
		case '&':
			//fprintf(stderr, "case: %c\n", arg[0]);		
//			ar.sel = 5;
			act = 5;
			break;
		case '<':
			//fprintf(stderr, "case: %c\n", arg[0]);		
//			ar.sel = 6;
			act = 6;
			break;
		case '|':
			//fprintf(stderr, "case: %c\n", arg[0]);		
//			ar.sel = 7;
			act = 7;
			break;
		default:
//			fprintf(stderr, "Default Entered\n");		
//			ar.sel = 0;
			act = 0;
			break;
	}
	


return act;
}
//This is where the arguments are run into execvp depending on the cases from isRedir()
int doarg(char **argv, char **argv2, int act)
{
       int fd;
       int fd2[2];
       pid_t pid, pid2;

//       fprintf(stderr, "argv[0]:%s, placement:%d, aact:%d, argv[3]:%s \n", argv[0], ar.rplace, act, argv[ar.rplace+1]);
       
       
       pid = fork();
       if(pid < 0)
       {
	 fprintf(stderr, "%s\n", "fork() failure");
	 exit(EXIT_FAILURE);
       }

        else if (pid == 0) // child process 
        { 
        	switch(act)
        	{

        		case 0:// no redirection or pipes
        			setpgid(0,0);          			
  	 			if(execvp(argv[0], argv) == -1)
				{
				   fprintf(stderr, "%s is not a valid command\n",argv[0]);
	 			   exit(EXIT_FAILURE);
       				}
				break;
      			case 1:// > or a 1> //cmd > file Redirect the standard output (stdout) of cmd to a file
	      			setpgid(0,0);
 //    				fprintf(stderr, "argv[0]:%s, a[1]:%s, argv[2]:%s \n", argv[0], argv[1], argv2[0]); 	   
             			fd = open(argv2[0], O_CREAT | O_WRONLY , S_IRUSR | S_IWUSR | S_IXUSR);
              			dup2(fd,1);          			
  	 			if(execvp(argv[0], argv) == -1)
				{
				   fprintf(stderr, "%s is not a valid command\n",argv[0]);
	 			   exit(EXIT_FAILURE);
	 			}
				break;   		
        		case 2://2>file Redirect the standard error (stderr) of cmd to a file. 2 is the default fd for stderr.
				setpgid(0,0);
//				fprintf(stderr, "argv[0]:%s, a[1]:%s, argv[2]:%s \n", argv[0], argv[1], argv2[0]); 
             		        fd = open(argv2[0], O_CREAT | O_WRONLY , S_IRUSR | S_IWUSR | S_IXUSR);
              			dup2(fd,2);        	
  	 			if(execvp(argv[0], argv) == -1)
				{
				   fprintf(stderr, "%s is not a valid command\n",argv[0]);
	 			   exit(EXIT_FAILURE);
	 			}
	 			break;               	
        		case 3://>>file Append stdout of cmd to a file.
				setpgid(0,0);
//				fprintf(stderr, "argv[0]:%s, a[1]:%s, argv[2]:%s \n", argv[0], argv[1], argv2[0]); 
                		fd = open((char*)argv2[0], O_CREAT | O_WRONLY | O_APPEND , S_IRUSR | S_IWUSR | S_IXUSR);
               			dup2(fd,1);        		
  	 			if(execvp(argv[0], argv) == -1)
				{
				   fprintf(stderr, "%s is not a valid command\n",argv[0]);
	 			   exit(EXIT_FAILURE);
	 			}
                		break;
        		case 4://2>>file Append stderr of cmd to a file.
				setpgid(0,0);
        			fd = open(argv2[0], O_CREAT | O_WRONLY | O_APPEND , S_IRUSR | S_IWUSR | S_IXUSR);
                		dup2(fd,2);
  	 			if(execvp(argv[0], argv) == -1)
				{
				   fprintf(stderr, "%s is not a valid command\n",argv[0]);
	 			   exit(EXIT_FAILURE);
	 			}
               			break;
        		case 5://&file Redirect stdout and stderr of cmd to a file
				setpgid(0,0);
				fd = open(argv2[0], O_CREAT | O_WRONLY , S_IRUSR | S_IWUSR | S_IXUSR);
				dup2(fd,1);
				dup2(fd,2);
  	 			if(execvp(argv[0], argv) == -1)
				{
				   fprintf(stderr, "%s is not a valid command\n",argv[0]);
	 			   exit(EXIT_FAILURE);
	 			}      
				break;		
        		case 6:// <file Redirect the contents of the file to the standard input (stdin) of cmd.
				setpgid(0,0);
				fd = open(argv2[0], O_CREAT | O_RDONLY , S_IRUSR | S_IWUSR | S_IXUSR);
				dup2(fd,0);
  	 			if(execvp(argv[0], argv) == -1)
				{
				   fprintf(stderr, "%s is not a valid command\n",argv[0]);
	 			   exit(EXIT_FAILURE);
	 			}
				break;    		
        		case 7:// |cmd1 | cmd2 Redirect stdout of cmd1 to stdin of cmd2.
				setpgid(0,0);
				pipe(fd2);
				pid2 = fork();
				if (pid2 < 0){
				    printf("Fork2 Failed\n");
				}
				else if (pid2 == 0){
				    dup2(fd2[1],1);
				    close(fd2[0]);
  	 			    if(execvp(argv[0], argv) == -1)
				    {
				       fprintf(stderr, "%s is not a valid command\n",argv[0]);
	 			       exit(EXIT_FAILURE);
	 		   	    }
				}
				else 
				{
				    dup2(fd2[0],0);
				    close(fd2[1]);  
  	 			    if(execvp(argv2[0], argv2) == -1)
				    {
				       fprintf(stderr, "%s is not a valid command\n",argv[0]);
	 			       exit(EXIT_FAILURE);
	 		   	    }
				}   
				break;  		
        		//default:
        		
		}
/*	
		if(execvp(argv[0], argv) == -1)
		{
		   fprintf(stderr, "%s is not a valid command\n",argv[0]);
		   exit(EXIT_FAILURE);
		}
*/
	}
	else if (act != 5){ // parent process 
//          printf("running parent\n");
            pid_t child_pid;
            int status;
            child_pid = wait(&status); //wait the child process to finish
            if (child_pid < 0)
     	    {
	       fprintf(stderr, "%s\n", "wait() error");
	       exit(EXIT_FAILURE);
            }
        }
     return 0;   
}

//Print out all the arguments
void print_args(char **args)
{
   int i;
   printf("Command: %s\n", args[0]);
   printf("%s","Arguments:\n");
   for(i=1; args[i]; i++)
   {
      printf("argv[%d]: ", i);
      printf("%s\n", args[i]);
   }
}

int main(void)
{

   char  line[MAX_LINE];
   char  *line_res;
   char **argv;
   int act = 0;
   int atest = 0;
   char **argv2;
   char **args;
   int argc;
   args = malloc(MAX_LINE *sizeof(char*));
   argv = malloc(MAX_LINE *sizeof(char*));
   argv2 = malloc(MAX_LINE *sizeof(char*));
   while(1)
   {

      fprintf(stderr,"%s","$ ");
      line_res = fgets(line, MAX_LINE, stdin);
      if(!line_res)
      {	 break;  }
      

        int len = strlen(line);
        if( len > 0 && line[len-1] == '\n')
        {   line[len-1] = '\0';	}
        
   	//use strtok function to split the arguments up    
        int argc = 0;
        char *token;
        token = strtok(line," ");
        while (token != NULL) {
            args[argc] = token;
            token = strtok (NULL," ");
            argc++;
        }
      

      print_args(args);

      int ii = 0;
      char *chktest;
      int final = 0;
      int nulpos = 0;
      char *temp1;     
      int background = 0;
      int kk = 0;

      //Go through the arguments. If there is a special character such as redirection, it will seperate the arguments into two lists.
      for(ii=0; ii<argc; ii++)
      {
//         fprintf(stderr,"argc:%s\n",args[ii]);
         act = isRedir(args[ii]);
 //        fprintf(stderr," arg count=%d__act =%d\n",argc, act);
      	 if(act == 0)
    	 {
 //     	   fprintf(stderr,"ii is:%d, sel=%d\n",ii, act);
	   chktest = args[ii];
           argv[ii] = chktest;
 //  	   fprintf(stderr,"argv:%s, sel=%d\n",argv[ii], act);
		if(final == 0)
		{ nulpos++;	}
       	 }
      	 else
      	 {
//		fprintf(stderr,"FINAL argv%s__________act:%d____\n",argv[ii], act);
       		if(final == 0)
		{
			final = act;
			break;
		}
      	 }
      }
      //Begin second list from where the special character left off. Can only handle redirects/pipes one at a time currently.
      for (ii = nulpos+1; ii < argc; ii++)
      {
	temp1 = args[ii];
	argv2[kk] = temp1;
//   	   fprintf(stderr,"In second for argv2:%s, sel=%d\n",argv2[kk], act);
	kk++;
      }
              
//      fprintf(stderr,"%s","starting doarg ");
      doarg(argv, argv2, final);
 
     }

   return EXIT_SUCCESS;
}
//-----------------------------------------------------------------------------





