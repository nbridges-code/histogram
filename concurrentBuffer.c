//
// defines a concurrent buffer that supports the producer-consumer pattern
//
// the buffer elements are simply void* pointers
// note: the buffer does not make a copy of the data.
//       it simply stores a pointer to the data.
//
// the buffer is FIFO (first in, first out)
//

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "concurrentBuffer.h"

// Node structure
typedef struct node_t{
  void * data;
  struct node_t* next;
} node_t;

// Control structure for handles
typedef struct info_t{
  pthread_mutex_t mu;
  pthread_cond_t cv;
  int capacity;
  int currentLength;
  node_t* head;
} info_t;

//static node_t* head;   // NOT a dummy head node: the first element in queue

// create a concurrent buffer
//   size: the number of elements in the buffer
//   returns: a handle to be used to operate on the buffer
void *createConcurrentBuffer(unsigned int size)
{
  info_t *handle = malloc(sizeof(info_t));
  if(handle == NULL){
    return NULL;
  }
  handle->capacity = size;
  handle->currentLength = 0;
  handle->head = NULL;//malloc(sizeof(node_t));
  if (pthread_mutex_init(&handle->mu, NULL) != 0){
    fprintf(stderr, "can't init mutex");
  }
  if (pthread_cond_init(&handle->cv, NULL) != 0){
    fprintf(stderr, "can't init condition variable");
  }
  return (void *) handle;
}

// put a value in a buffer
//   handle: handle for a concurrent buffer
//   p: pointer to be put in the buffer
//   note: calling thread will block until there is space in the buffer
void putConcurrentBuffer(void *handle, void *p)
{
  if(handle == NULL){
    fprintf(stderr, "Invalid handle\n");
    exit(-1);
  }
  info_t * q = (info_t *) handle;
  if (pthread_mutex_lock(&q->mu) != 0){
    fprintf(stderr, "Error in mutex_lock");
  }
  while(q->currentLength == q->capacity){
    if(pthread_cond_wait(&q->cv, &q->mu) != 0){
      fprintf(stderr, "Error in cond_wait\n");
    }
  }

  node_t * ptr = q->head;
  node_t * temp = malloc(sizeof(node_t));
  temp->data = p;
  temp->next = NULL;
  if(q->currentLength == 0){
    q->head = temp;
  }else{
    while(ptr->next){
      ptr = ptr->next;
    }
    ptr->next = temp;
  }

  q->currentLength = q->currentLength + 1;

  //if(q->currentLength == q->capacity){
  if(pthread_cond_signal(&q->cv) != 0){
    fprintf(stderr, "Error in cond_signal\n");
  }
  //}
  if (pthread_mutex_unlock(&q->mu) != 0){
    fprintf(stderr, "Error in mutex_unlock");
  }
}

// get a value from a buffer
//   handle: handle for a concurrent buffer
//   returns: pointer retrieved from buffer
//   note: calling thread will block until there is a value available
void *getConcurrentBuffer(void *handle)
{
  if(handle == NULL){
    fprintf(stderr, "Invalid handle\n");
    exit(-1);
  }
  info_t * q = (info_t *) handle;
  if (pthread_mutex_lock(&q->mu) != 0){
    fprintf(stderr, "Error in mutex_lock");
  }
  while(q->currentLength == 0){
    if(pthread_cond_wait(&q->cv, &q->mu) != 0){
      fprintf(stderr, "Error in cond_wait\n");
    }
  }


  node_t * cur = q->head;
  void * temp = q->head->data;
  q->head = q->head->next;
  free(cur);

  q->currentLength = q->currentLength - 1;

  //if(q->currentLength == 0){
  if(pthread_cond_signal(&q->cv) != 0){
    fprintf(stderr, "Error in cond_signal\n");
  }
  //}
  if (pthread_mutex_unlock(&q->mu) != 0){
    fprintf(stderr, "Error in mutex_unlock");
  }

  return temp;
}

// delete a buffer
//   handle: handle for the concurrent buffer to be deleted
void deleteConcurrentBuffer(void *handle)
{
  info_t * h = (info_t *) handle;
  node_t * ptr = h->head;
  node_t * temp;
  while(ptr){
    temp = ptr->next;
    free(ptr);
    ptr = temp;
  }
  free(h->head);
  free(h);
}
