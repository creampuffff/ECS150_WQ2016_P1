#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define INDEX 512

void search(DIR *dirPtr, char *directory, char *fileName, FILE *pf)
{
  struct dirent *d;
  struct stat buffer;
  DIR *dirPtrNext;
  int length = 0;
  int count = 0;

  // Read all items in directory
  while(d = readdir(dirPtr))
  {
    // skip . and .., otherwise it will loop back to current directory itself
    if( strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
      continue;

    length = strlen(directory) + strlen(d->d_name) + 1;
    count = strlen(directory);
  
    char fullDir[length+1];
 
    strcpy(fullDir, directory);
    // if last character is already /, not need to append one
    if(fullDir[count-1] == '/')
    {
      strcat(fullDir, d->d_name);
    }
    else
    {
      strcat(fullDir, "/");
      strcat(fullDir, d->d_name);
    }

    //store the fullDir into buffer
    if(stat(fullDir, &buffer) != 0)
    {
      perror("Appending Error");
      exit(0);
    }
    // If directory then enter and search
    if(S_ISDIR(buffer.st_mode))
    {
      printf("%s\n", fullDir);

      if((dirPtrNext = opendir(fullDir)) == NULL)
      {
        perror("Unable to open sub-directory");
        exit(0);
      }
      search(dirPtrNext, fullDir, fileName, pf);
    }
    else
    {
      printf("%s\n", fullDir);
    }

    if(strcmp(d->d_name, fileName) == 0)
    {
      fprintf(pf, "%s is found at %s\n", fileName, fullDir);
    }
  }
  closedir(dirPtr);
}

int main(int argc, char ** argv){

  char *directory = argv[1];  
  char *fileName = argv[2];
  char arr[INDEX];
  DIR *dirPtr;     // valid directory stream pointer
  int pfd[2];      //for pipe
  int TorF = 0;
  pipe(pfd);       //creat pipe with file discriptor pfd
  
  if(argc < 2)// no directory provided
  {
    printf("no directory is provided!\n");
    exit(0);
  }
  else if(argc < 3)
  {
    printf("no file name is provided!\n");
    exit(0);
  }

  if((dirPtr = opendir(directory)) == NULL)
  {
    perror("Unable to open entered directory");
    exit(0);
  }

  // The fdopen() function associates a stream with the existing file descriptor, fd. the mode of the stream in this case "w"
  // the result of applying fdopen() to a shared memory object is undfined.
  FILE *pf = fdopen(pfd[1], "w"); 
  search(dirPtr, directory, fileName, pf);  //call the search function
  fclose(pf);             //close the file descrtpter
  close(pfd[1]);          //turn off the output pipe file discriptor

  printf("\nSearch Result: \n"); //print search result stored in pipe;

  while((TorF = read(pfd[0], arr, INDEX)) > 0)
  {
    write(1, arr, TorF);
  }

  return 0;
}
