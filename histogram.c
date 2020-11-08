#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include "concurrentBuffer.h"

// allow this to be overridden at compile time
// e.g. gcc -DBUFFER_SIZE=1 ...
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 5
#endif

// struct to be used to pass information to the first thread
typedef struct info_t{
  char *file;
  void *buffer;
} info_t;

// the work functions for the threads
static void *work1(void *);
static void *work2(void *);

static int fileCount;
int lengths[45];
info_t * infoList[100];

pthread_mutex_t mu;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "Usage: histogram files...\n");
    exit(-1);
  }

  fileCount = argc-1;

  for(int i = 0; i < 45; i++){
    lengths[i] = 0;
  }

  for(int i = 0; i < fileCount; i++){
    info_t * ip = malloc(sizeof(info_t));
    if (ip == NULL)
    {
      fprintf(stderr, "can't malloc the info_t struct\n");
      exit(-1);
    }
    ip->file = argv[i+1];
    ip->buffer = createConcurrentBuffer(10);
    infoList[i] = ip;
  }




  pthread_t consumerList[fileCount];
  for(int i = 0; i < fileCount; i++){
    if (pthread_create(&consumerList[i], NULL, work1, (void *) infoList[i]) != 0){
      fprintf(stderr, "can't create thread 1\n");
      exit(-1);
    }
  }

  pthread_t producerList[fileCount];
  for(int i = 0; i < fileCount; i++){
    if (pthread_create(&producerList[i], NULL, work2, (void *) infoList[i]) != 0){
      fprintf(stderr, "can't create thread 2\n");
      exit(-1);
    }
  }


  for(int i = 0; i < fileCount; i++){
    if (pthread_join(producerList[i], NULL))
    {
      fprintf(stderr, "error in thread join");
    }
  }
  for(int i = 0; i < fileCount; i++){
    if (pthread_join(consumerList[i], NULL))
    {
      fprintf(stderr, "error in thread join");
    }
  }

  for(int i = 0; i <= 44; i++){
    printf("%d %d\n", i+6, lengths[i]);
  }
}

#define MAX_LINE_LENGTH 1024
static char *getLine(FILE *fp, int * count)
{
  char buf[MAX_LINE_LENGTH];
  int i = 0;
  int c = getc(fp);
  if (c == EOF) return NULL;
  while (c != EOF && c != '\n')
  {
    buf[i] = c;
    i += 1;
    if (i == MAX_LINE_LENGTH)
    {
      fprintf(stderr, "maximum line length (%d) exceeded\n", MAX_LINE_LENGTH);
      exit(-1);
    }
    c = getc(fp);
  }
  if (c == '\n')
  {
    buf[i] = c;
    i += 1;
  }
  buf[i] = 0;
  *count = i;
  char *s = malloc(i+1);
  if (s == NULL)
  {
    fprintf(stderr, "malloc failed in getLine\n");
    exit(-1);
  }
  strcpy(s, buf);
  return s;
}

// work function for the first thread (the producer)
static void *work1(void *info)
{
  info_t *ip = info;

  FILE *fp = fopen(ip->file, "r");

  int charCount = 0;
  int lineCount = 0;
  char * block = getLine(fp, &lineCount);
  char * temp;

  char *line;
  int flag = 1;
  while(flag){
    // Get block
    while (charCount < 1000){
      charCount += lineCount;
      line = getLine(fp, &lineCount);
      if(line != NULL){
        temp = (char *) malloc(strlen(block) + strlen(line) + 1);
        strcpy(temp, block);
        strcat(temp, line);
        free(line);
        block = temp;
        //free(temp);
      }else{
        // EOF
        flag = 0;
        break;
      }
    }
    charCount = 0;

    // Pass block to buffer
    putConcurrentBuffer(ip->buffer, block);
    block = " ";
  }

  // put a NULL to indicate EOF
  putConcurrentBuffer(ip->buffer, NULL);

  return NULL;
}

// work function for the second thread (the consumer)
static void *work2(void *info)
{
  info_t *ip = info;

  // get lines from the buffer
  // and sum the number of characters in the line
  char *line;
  int currentCount = 0;
  while ((line = getConcurrentBuffer(ip->buffer)) != NULL)
  {
    // Count the words here
    for(int i = 0; i < strlen(line); i++){
      if(isalpha(line[i])){
        currentCount += 1;
      }else{
        // End of word
        if(currentCount > 5 && currentCount < 50){
          if (pthread_mutex_lock(&mu) != 0){
            fprintf(stderr, "error in mutex_lock in child");
          }

          lengths[currentCount-6] = lengths[currentCount-6] + 1;

          if (pthread_mutex_unlock(&mu) != 0){
            fprintf(stderr, "error in mutex_unlock in child");
          }
        }
        currentCount = 0;
      }
    }
    free(line);
  }

  // return the character count via the void* return value
  return (void *) 1;
}
