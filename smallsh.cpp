#include "smallsh.h" /* include file for example */
#include <stdlib.h>
#include <signal.h>

static int c_pid;
static int flag = 0;

void action (int sig)
{
  int *status;

  if(c_pid)   // if c_pid != 0, which means the foreground exist then we can kill it
  {    
    kill(SIGKILL, c_pid);
    printf("\nKilled the foreground process successfully :)\n");
    fflush(stdout);
    waitpid(c_pid, status, 0);
  }
  else        // otherwise it has nothing to kill
  {
    flag = 1;
    printf("\nTry to interrupt but no foreground process to kill :(\n");
    fflush(stdout);  
  }
}

/* program buffers and work pointers */
static char inpbuf[MAXBUF], tokbuf[2*MAXBUF], *ptr = inpbuf, *tok = tokbuf;

userin(p) /* print prompt and read a line */
char *p;
{
  int c, count;
  /* initialization for later routines */
  ptr = inpbuf;
  tok = tokbuf;

  /* display prompt */
  printf("%s ", p);

  for(count = 0;;){
    if((c = getchar()) == EOF) 
    {
      if(flag) // if it needs to ignore the terminate
      {
        flag = 0; // reset the flag to false
        c = '\n'; // end the line with \n
      } 
      else    // if just need to terminate
      {
        return(EOF); // terminate current process, but it never ends the main process then
      }
    }

    if(count < MAXBUF)
      inpbuf[count++] = c;

    if(c == '\n' && count < MAXBUF){
      inpbuf[count] = '\0';
      return(count);
    }

    /* if line too long restart */
    if(c == '\n'){
      printf("smallsh: input line too long\n");
      count = 0;
      printf("%s ", p);
    }
  }
}

static char special[] = {' ', '\t', '&', ';', '\n', '\0'};

inarg(c) /* are we in an ordinary argument */
char c;
{
  char *wrk;
  for(wrk = special; *wrk != '\0'; wrk++)
    if(c == *wrk)
      return(0);

  return(1);
}

gettok(outptr) /* get token and place into tokbuf */
char **outptr;
{
  int type;

  *outptr = tok;
  /* strip white space */
  for(;*ptr == ' ' || *ptr == '\t'; ptr++)
    ;

  *tok++ = *ptr;

  switch(*ptr++){
    case '\n':
      type = EOL; break;
    case '&':
      type = AMPERSAND; break;
    case ';':
      type = SEMICOLON; break;
    default:
      type = ARG;
      while(inarg(*ptr))
        *tok++ = *ptr++;
  }

  *tok++ = '\0';
  return(type);
}

/* execute a command with optional wait */
runcommand(cline, where)
char **cline;
int where;
{
  int pid, exitstat, ret;

  if((pid = fork()) < 0){
    perror("smallsh");
    return(-1);
  }

  if(pid == 0){
    if (where == BACKGROUND) // make the background process as leader / parent
      setsid();  // creates a new session if the calling process is not a process group leader

    execvp(*cline, cline);
    perror(*cline);
    exit(127);
  }

  /* code for parent */
  /* if background process print pid and exit */
  if(where == BACKGROUND){
    printf("[Process id %d]\n", pid);
    return(0);
  }

  c_pid = pid;  // current pid
  /* wait until process pid exits */
  while( (ret=wait(&exitstat)) != pid && ret != -1)
    ;
  c_pid = 0;    // reset c_pid

  return(ret == -1 ? -1 : exitstat);
}

procline() /* process input line */
{
  char *arg[MAXARG+1]; /* pointer array for runcommand */
  int toktype; /* type of token in command */
  int narg; /* numer of arguments so far */
  int type; /* FOREGROUND or BACKGROUND? */

  for(narg = 0;;){ /* loop forever */
    /* take action according to token type */
    switch(toktype = gettok(&arg[narg])){
      case ARG:
        if(narg < MAXARG)
          narg++;
        break;

      case EOL:
      case SEMICOLON:
      case AMPERSAND:
        type = (toktype == AMPERSAND) ? BACKGROUND : FOREGROUND;

        if(narg != 0){
          arg[narg] = NULL;
          runcommand(arg, type);
        }
        
        if(toktype == EOL)
          return;

        narg = 0;
        break;
    }
  }
}

char *prompt = "Command>"; /* prompt */

main()
{
  static struct sigaction act;
  // we want a process to ignore the interrupt signal SIGINT and SIGQUIT
  act.sa_handler = action;
  sigaction(SIGINT, &act, NULL);
  sigaction(SIGQUIT, &act, NULL);

  while(userin(prompt) != EOF)
      procline();
}
