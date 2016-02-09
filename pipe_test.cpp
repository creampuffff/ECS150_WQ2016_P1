#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define INDEX 1024

void reverse(char str[], int count)
{
  int i, j = 0;
  char temp;
  
  for(i = 0, j = count-1; i <= count/2 -1 ; i++, j--) {
    temp = str[i];
    str[i] = str[j];
    str[j] = temp;
  }
}

void uppercase(char str[], int count)
{
  char temp[count+1];
  int i = 0;

  for(i = 0; i < count+1; i++)
  {
    if(isdigit(str[i]))
    {
      temp[i] = str[i];
    }
    else
    {
      temp[i] = toupper(str[i]);
    }
  }
  
  strcpy(str, temp);
}

int main()
{
  char chr;
  int count = 0;
  int i = 0;
  int p1id = 0;
  int p2id = 0;
  int p3id = 0;
  int *status;
  char str[count+1];
  int p1to2fd[2], p2to1fd[2], p2to3fd[2], p3to2fd[2];
  pipe(p1to2fd);
  pipe(p2to1fd);
  pipe(p2to3fd);
  pipe(p3to2fd);
  
  printf("I am process P1\n");
  p1id = getpid();
  printf("My pid is %d\n", p1id);
  printf("Please enter a string: ");
  fflush(stdout); // still in buffer, then print right away

  while(1)
  {
    read(0, &chr, 1);
    if(chr == '\n')
    {
      write(p1to2fd[1], "\0", 1);
      break;
    }
    count++;
    write(p1to2fd[1], &chr, 1); // write, p1fd[1] store the output
  }
  
  if((p2id = fork()) == 0)
  {
    close(p1to2fd[1]);
    read(p1to2fd[0], str, count+1); // read from p1fd[0] which represent input 

    printf("\nI am process P2\n");
    p2id = getpid();
    printf("My pid is %d\n", p2id);
    printf("I just received the string: %s\n", str);
    close(p1to2fd[0]);

    reverse(str, count);
    printf("I am sending %s to P3\n", str);
    write(p2to3fd[1], str, count+1);

    if((p3id = fork()) == 0)
    {
      close(p2to3fd[1]);
      read(p2to3fd[0], str, count+1);

      printf("\nI am process P3\n");
      p3id = getpid();
      printf("My pid is %d\n", p3id);
      printf("I just received the string: %s\n", str);
      
      uppercase(str, count);
      printf("I am sending %s to P1\n", str);
      write(p3to2fd[1], str, count+1);
      close(p2to3fd[0]);
      
      printf("\nI am process P3\n");
      p3id = getpid();
      printf("My pid is %d\n", p3id);
      printf("I am about to die...\n");

      exit(0);
    }

    if((p3id != 0))
    {
      wait(status);
      close(p3to2fd[1]);
      read(p3to2fd[0], str, count+1);
      close(p3to2fd[0]);

      write(p2to1fd[1], str, count+1);

      printf("\nI am process P2\n");
      p2id = getpid();
      printf("My pid is %d\n", p2id);
      printf("I am about to die...\n");

      exit(0);
    }
  }

  if(p2id != 0)
  {
    wait(status);
    close(p2to1fd[1]);
    read(p2to1fd[0], str, count+1);

    printf("\nI am process P1\n");
    p1id = getpid();
    printf("My pid is %d\n", p1id);
    printf("I just received the string: %s\n", str);
    close(p2to1fd[0]);

    printf("\nI am process P1\n");
    p1id = getpid();
    printf("My pid is %d\n", p1id);
    printf("I am about to die...\n");

    exit(0);
  }

  return 0;
}

/*
Sample Output:
I am process P1
My pid is 8533
Please enter a string: Hi, professor!

I am process P2
My pid is 8554
I just received the string: Hi, professor!
I am sending !rosseforp ,iH to P3

I am process P3
My pid is 8555
I just received the string: !rosseforp ,iH
I am sending !ROSSEFORP ,IH to P1

I am process P3
My pid is 8555
I am about to die...

I am process P2
My pid is 8554
I am about to die...

I am process P1
My pid is 8533
I just received the string: !ROSSEFORP ,IH

I am process P1
My pid is 8533
I am about to die...
*/
