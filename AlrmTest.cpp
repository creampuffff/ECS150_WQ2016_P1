#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static int flag = 0; // Global declarations

void c_action(int sig){
  flag = 1;
}

int main()
{
  int count = 0;
  int i = 0;
  int p1id = 0;
  int p2id = 0;
  
  struct sigaction c_act;
  c_act.sa_handler = c_action;
  sigaction(SIGALRM, &c_act, NULL);
  
  if((p1id = fork()) == -1)
  {
    printf("Error in 1st fork in AlrmTest\n");
  }
  else if(p1id == 0)
  {
    p1id = getpid();
    for(;;)
    {
      if(flag)
      {
        printf("Count is %d, my pid is %d\n", count, p1id);
        fflush(stdout);
        count = 0;
        flag = 0;
      }
      count++;
    } 
  }
  else if ((p2id = fork()) == -1)
  {
    printf("Error in 2nd fork in AlrmTest\n");
  }
  else if(p2id == 0)
  {
    p2id = getpid();
    for(;;)
    {
      if(flag)
      {
        printf("Count is %d, my pid is %d\n", count, p2id);
        fflush(stdout);
        count = 0;
        flag = 0;
      }
      count++;
    } 
  }
  else
  {
    for(i = 0; i < 5; i++)
    {
      sleep(1);
      kill(p1id, SIGALRM);
      kill(p2id, SIGALRM);
    }  
  }

  return 0;
}

/*
Sample Output:
Count is 631253771, my pid is 2451
Count is 631336081, my pid is 2452
Count is 631277094, my pid is 2451
Count is 631366718, my pid is 2452
Count is 631309104, my pid is 2451
Count is 631384233, my pid is 2452
Count is 631269531, my pid is 2451
Count is 631360318, my pid is 2452
Count is 631419103, my pid is 2452
Count is 631345350, my pid is 2451
*/
