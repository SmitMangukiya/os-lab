/* (1) filename          */
/* minishell.c           */

/* (2) last name, first name */
/* Mangukiya Smit */
/* ID - 201801191 */

/* (3) appropriate includes */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h> 
#include<unistd.h>
#include<fcntl.h>

int main()
{
  while( 1 )
  {
/* ==== DO NOT MODIFY ANY OF THESE DECLARATIONS ============ */
    char cmdline[1024];  // the entire minishell command line
    int  num_tokens = 0; // number of tokens on the minishell command line
    char tokens[3][256]; // an array of the tokens
    int i = 0;           // just a counter
    char *p;             // pointer for stepping thru tokens
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
    /* (4) Print the minishell prompt: */
    printf("IE411> ");

/* ==== DO NOT MODIFY THIS NEXT SECTION OF CODE ============ */
    /* This reads an entire line from stdin: */
    if( fgets( cmdline, 1024, stdin ) == NULL ) { printf("\n"); break; }

    // This replaces the newline at the end of the string with a NULL
    *strrchr( cmdline, '\n') = '\0';

    // extract the individual tokens from the minishell command line
    p = cmdline;
    while( 1 == sscanf( p, " %s", tokens[i] ) )
    {
      p = strstr( p, tokens[i] ) + strlen( tokens[i] );
      ++i;
    }
    num_tokens = i;
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

    /* (5) Handle the case where there were no tokens on the
     * minishell command line, i.e., maybe the user just
     * hit the ENTER key:
     */

    if(cmdline[0] == '\0'){
      continue;
    }

    /* (6) Handle the case where something OTHER than "mmame" */
    /*     was entered on the minishelrl command line:         */

    char* cmdname = "mmame";
    if(strcmp(cmdname, tokens[0]) != 0){
      printf("Command \'%s\' not found :\n", tokens[0]);
      continue;
    }

    /* (7) Something valid entered on the command line, gotta fork */
    pid_t pid = fork();
    /* (8) Handle the case where the fork failed: */
    if(pid == -1 )
    {
      exit( 1 );
    } 

    /* (9) Code that the parent executes: */
    if(pid !=  0)
    {
      // parent waits for child (mmame) to terminate
      int status;
      wait(&status);
    }
    /* (10) Code that the child executes: */
    else
    {
      /* (11) The child must handle these cases: */

      /*      Part 1: mmame                      */
      if(num_tokens == 1){
        char *args[]={"./mmame",NULL}; 
        execvp(args[0],args);
      }
      
      /*      Part 2: mmame  filename           */
      if(num_tokens == 2){
        char *args[]={"./mmame",tokens[1],NULL};
        execvp(args[0], args);
      }
	 
      /*      Part 3: mmame < filename           */
      if(tokens[1][0] == '<'){
        int fd = open(tokens[2], O_CREAT | O_RDWR, 0777);
        dup2(fd, STDIN_FILENO);
        close(fd);
        char *args[]={"./mmame", NULL};
        execvp(args[0], args);
      }
      
      /*      Part 4: mmame > filename           */
      if(tokens[1][0] == '>'){
        int fd = open(tokens[2], O_RDWR | O_CREAT | O_TRUNC, 0777);
        dup2(fd, STDOUT_FILENO);
        close(fd);
        char *args[]={"./mmame",NULL}; 
        execvp(args[0],args);
      }

    }
  }
  return 0;
}
