#include "server_impl.h"
#include "request.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

int *buffer;
int buffernum;
int i;
int threadnum;
int bufferfull;
int out;
int in;

pthread_t *threads;
sem_t empty, full, mutex;
void *consumer();

void server_init(int argc, char* argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s server " 
    "[portnum] [threads] [buffers]\n", argv[0]);
    exit(1);
  }
  if(atoi(argv[3]) <= 0)
    exit(1);
  if(atoi(argv[2]) <= 0)
    exit(1);
  buffernum = atoi(argv[3]);
  threadnum = atoi(argv[2]);
  buffer = malloc(sizeof(uint) * buffernum);
  for (i = 0; i < buffernum; i++)
    buffer[i] = -1;
  bufferfull = 0;
  in = 0;
  out = 0;
  threads = (pthread_t *)malloc(sizeof(pthread_t) * threadnum);
  if (sem_init(&empty, 0, buffernum) != 0) 
  	exit(1);
  if (sem_init(&full, 0, 0) != 0)
  	exit(1);
  if (sem_init(&mutex, 0, 1) != 0)
  	exit(1);
  for (i = 0; i < threadnum; i++) {
    if (pthread_create(&threads[i], NULL, consumer, NULL) != 0)
          exit(1);
  }
}

void *consumer() {
  int fd_out = 0;
  while (1) {
    sem_wait(&full);
    sem_wait(&mutex);
    fd_out = buffer[out];
    buffer[out] = -1;
    out = (out + 1) % buffernum;
    bufferfull = bufferfull - 1;
    sem_post(&mutex);
    sem_post(&empty);
    requestHandle(fd_out);
  }
}

void server_dispatch(int connfd) {
  int fd_in = connfd;
  sem_wait(&empty);
  sem_wait(&mutex);
  buffer[in] = fd_in;
  in = (in + 1) % buffernum;
  bufferfull = bufferfull + 1;
  sem_post(&mutex);
  sem_post(&full);
}
