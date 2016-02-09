#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>

void printOutput(int pNum)
{
  printf("I am process %d; my process pid is %d; my parent's pid is %d.\n", pNum, getpid(), getppid());
}

void processTree(int n, int top)
{
  int pidL;
  int pidR;
  int left = top*2;
  int right = left+1;
  
  if(n == 1)
  {
    exit(0);
  }
  else
  {
    if((pidL = fork()) == 0)
    {
      printOutput(left);
      processTree(n-1, left);
    }
    else if((pidR = fork()) == 0)
    {
      printOutput(right);
      processTree(n-1, right);
    }
    else if(pidL > 0 && pidR > 0)
    {
      waitpid(pidL, NULL, 0);
      waitpid(pidR, NULL, 0);
    }
  }
}

int main(int argc, char ** argv)
{
  int n = atoi(argv[1]);
  int top = 1; // 1st node

  if(argc < 2)
  {
    printf("depth n is not provided!\n");
    exit(0);
  }
  if(n == 0)
  {
    exit(0);
  }

  printOutput(top);
  processTree(n, top);
  return 0;
}

