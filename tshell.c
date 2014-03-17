#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <dirent.h>
#include <errno.h>
#define MAX_LINE 1024

struct arguments{
	int f;// |
	int l; // <
	int p;//  &
	int s; //  >
	int c;
	char *fval;
	char *pval; 	
	char *sval; 	
} ar;
typedef int Myfunc(const char *, const struct stat *, int);
static Myfunc myfunc;
int myftw(char *, Myfunc *);
int getPaths(Myfunc *);
static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;
#define FTW_F 1 /* file other than directory */
#define FTW_D 2 /* directory */
#define FTW_DNR 3 /* directory that can’t be read */
#define FTW_NS 4 /* file that we can’t stat */   
static size_t pathlen;


int count_words(char* line)
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
   int argc = count_words(line);
   ar.c = argc;
   int i;
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
      strcpy(argv[i], line);

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

int printdir(Myfunc* func){
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	int ret, n, fnlen;
	if (lstat(ar.pval, &statbuf) < 0)
		return(func(ar.pval, &statbuf,FTW_NS));/* stat error */

	if (S_ISDIR(statbuf.st_mode) == 0)
		return(func(ar.pval, &statbuf,FTW_F));/* not a directory */

	/*
	* It’s a directory. First call func() for the directory,
	* then process each filename in the directory.
	*/
	if ((ret = func(ar.pval, &statbuf, FTW_D)) != 0)
		return(ret);
	n = strlen(ar.pval);	
	if (n + NAME_MAX + 2 > pathlen) {
	/* expand path buffer */
		pathlen *= 2;
		if ((ar.pval = realloc(ar.pval, pathlen)) == NULL)
			 fprintf(stderr, "%s\n", "Alloc failed");
	
	}
	ar.pval[n++] = '/';
	ar.pval[n] = 0;
	if ((dp = opendir(ar.pval)) == NULL)	// can’t read directory 
		return(func(ar.pval, &statbuf, FTW_DNR));
	fprintf(stdout, "Dir: %s\n", ar.pval); //print directory name

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
	    fprintf(stderr, "%s\n", "execvp failure");
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


int gopt(int argc, char **argv)
{
       ar.l = 0;       
       ar.f = 0;       ar.fval = NULL;
       ar.p = 0;       ar.pval = NULL;
       ar.s = 0;       ar.sval = NULL;                  
       int index;
       int c;  
       opterr = 0;
       int filechk = 0;    

        
       while ((c = getopt (argc, argv, ">:|:&:<:")) != -1){
         switch (c)
           {
           case '>':
             ar.l = 1;       
             break;
           case '|':
             ar.fval = optarg;
           case '&':
             ar.pval = optarg;
           case '<':
             ar.s = 1;
             ar.sval = optarg;           
             break;
           case '?':
             if (optopt == '>')
               fprintf (stderr, "Option -%c requires an argument.\n", optopt);
             else if (optopt == '|')
               fprintf (stderr, "Option -%c requires an argument.\n", optopt);
             else if (optopt == '&')
               fprintf (stderr, "Option -%c requires an argument.\n", optopt);
//             else if (isprint (optopt))
//               fprintf (stderr, "Unknown option `-%c'.\n", optopt);
//             else
 //              fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
             return 1;
           default:
             abort ();
           } //end switch(c)
       }  //end while getopt

///       printf ("pflag = %d, fflag = %d, lflag = %d, sflag = %d\n",
//               ar.p, ar.f, ar.l, ar.s );    
//       printf ("fvalue = %s, pvalue = %s, svalue = %s\n",
 //              ar.fval, ar.pval, ar.sval );  
                       
       printf ("command: %s\n", argv[0]);
       //Print out arguments
       for (index = optind; index < argc; index++){
         printf ("argument[%d]: %s\n", index, argv[index]);
       }       
       
//	 myftw(ar.pval, myfunc);

       return 0;       
       
}
#ifdef  PATH_MAX
static int pathmax=PATH_MAX;
#else
static int pathmax=0;
#endif
#define PATH_MAX_GUESS 1024
char *path_alloc(int *size)
{
    /* return allocated size, if nonull */
    char *ptr;
    if(pathmax == 0){
        errno = 0;
    if((pathmax = pathconf("/", _PC_PATH_MAX)) < 0)
    {
        if(errno == 0)
            pathmax = PATH_MAX_GUESS;
        else
            printf("pathconf error for _PC_PATH_MAX");
    }
    }
    else
    {
        pathmax++;
    }
   
    if((ptr = malloc(pathmax + 1)) == NULL)
        printf("malloc error for pathname\n");
    if(size != NULL)
        *size = pathmax + 1;
    return (ptr);
}
//---------------------------------------------------------------------
myftw(char *pathname, Myfunc *func)
{
	ar.pval = (char*)path_alloc(&pathlen);
	/* malloc PATH_MAX+1 bytes */
	if (pathlen <= strlen(pathname)) {
		pathlen = strlen(pathname) * 2;
		if ((ar.pval = realloc(ar.pval, pathlen)) == NULL)
			 fprintf(stderr, "%s\n", "Alloc failed");
	}
	strcpy(ar.pval, pathname);
//	return(getPaths(func));
	return(printdir(func));
}
//---------------------------------------------------------------------
int getPaths(Myfunc* func){

	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	int ret, n, fnlen;
	if (lstat(ar.pval, &statbuf) < 0)
		return(func(ar.pval, &statbuf,FTW_NS));/* stat error */

	if (S_ISDIR(statbuf.st_mode) == 0)
		return(func(ar.pval, &statbuf,FTW_F));/* not a directory */

	/*
	* It’s a directory. First call func() for the directory,
	* then process each filename in the directory.
	*/
	if ((ret = func(ar.pval, &statbuf, FTW_D)) != 0)
		return(ret);
	n = strlen(ar.pval);	
	if (n + NAME_MAX + 2 > pathlen) {
	/* expand path buffer */
		pathlen *= 2;
		if ((ar.pval = realloc(ar.pval, pathlen)) == NULL)
			 fprintf(stderr, "%s\n", "Alloc failed");
	
	}
	ar.pval[n++] = '/';
	ar.pval[n] = 0;
	if ((dp = opendir(ar.pval)) == NULL)	// can’t read directory 
		return(func(ar.pval, &statbuf, FTW_DNR));
	fprintf(stdout, "Dir: %s\n", ar.pval); //print directory name
	while ((dirp = readdir(dp)) != NULL) {
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0){
		  continue;/* ignore dot and dot-dot */
		}		
		strcpy(&ar.pval[n], dirp->d_name); /* append name after "/" */
		fnlen = strlen(dirp->d_name) - 1;
		if(ar.f > 0){	//if the f flag was set
		  if(ar.pval[n+fnlen] == ar.fval[0]){	//match with chosen file type
//		    fprintf(stdout, "%s\n", ar.pval); // print full file pathname
//		    fprintf(stdout, "%s\n", dirp->d_name); // print name of file
	    
 		  }
 		}
 		else{
//		  fprintf(stdout, "%s\n", ar.pval); // print full file pathname

 		}

		if ((ret = getPaths(func)) != 0){	/* recursive */
		  fprintf(stderr, "Got getPaths != 0 %d %s\n", ret, dirp->d_name);
		  break; /* time to leave */
		}
	}
	ar.pval[n-1] = 0; /* erase everything from slash onward */
	if (closedir(dp) < 0){
	  fprintf(stderr, "%s\n", "Can't close directory");
	}
	return(ret);
  

}

//---------------------------------------------------------------------
int myfunc(const char *pathname, const struct stat *statptr, int type)
{
//	fprintf(stderr, "%s\n", pathname);
	switch (type) {
		case FTW_F:
		  switch (statptr->st_mode & S_IFMT) {
		    case S_IFREG:
		      nreg++;
		      break;
		    case S_IFBLK:
		      nblk++;
		      break;
		    case S_IFCHR:
		      nchr++;
		      break;
		    case S_IFIFO:
		      nfifo++;
		      break;
		    case S_IFLNK:
		      nslink++;
		      break;
		    case S_IFSOCK: 
		      nsock++;
		      break;
		    case S_IFDIR:
	              /* directories should have type = FTW_D */
		      fprintf(stderr, "for s_IFDIR for %s\n", pathname);
		  }
		break;
		case FTW_D:
		  ndir++;
		  break;
		case FTW_DNR:
		  fprintf(stderr, "%s\n", "Can't read directory");	
		  break;
		case FTW_NS:
		  fprintf(stderr, "%s %s\n", "stat error for", pathname);
		  break;
		default:
		  fprintf(stderr, "unknown type %d for pathname %s", type, pathname);
	}
	return(0);
}
//-----------------------------------------------------------------------------





