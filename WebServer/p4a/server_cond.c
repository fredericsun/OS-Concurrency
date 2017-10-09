#include "server_impl.h"
#include "request.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int *buffer;
int buffernum;
int i;
int threadnum;
int bufferfull;
int out;
int in;

pthread_cond_t empty;
pthread_cond_t full;
pthread_mutex_t mutex;
pthread_t *threads;
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
  bufferfull = 0;
  in = 0;
  out = 0;
  buffernum = atoi(argv[3]);
  threadnum = atoi(argv[2]);
  threads = (pthread_t *)malloc(sizeof(pthread_t) * threadnum);
  buffer = malloc(sizeof(uint) * buffernum);
  for (i = 0; i < buffernum; i++)
    buffer[i] = -1;
  if (pthread_cond_init(&empty, NULL) != 0) 
  	exit(1);
  if (pthread_cond_init(&full, NULL) != 0)
  	exit(1);
  if (pthread_mutex_init(&mutex, NULL) != 0)
  	exit(1);
  for (i = 0; i < threadnum; i++) {
    if (pthread_create(&threads[i], NULL, consumer, NULL) != 0)
	  exit(1);
  }
}

void *consumer() {
  int fd_out = 0;
  while (1) {
    pthread_mutex_lock(&mutex);
    while (bufferfull == 0)
      pthread_cond_wait(&full, &mutex);
    fd_out = buffer[out];
    buffer[out] = -1;
    out = (out + 1) % buffernum;
    bufferfull = bufferfull - 1;
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
    requestHandle(fd_out);
  }
}
void server_dispatch(int connfd) {
  int fd_in = connfd;
  pthread_mutex_lock(&mutex);
  while (bufferfull == buffernum) 
    pthread_cond_wait(&empty, &mutex);
  buffer[in] = fd_in;
  in = (in + 1) % buffernum;
  bufferfull = bufferfull + 1;
  pthread_cond_signal(&full);
  pthread_mutex_unlock(&mutex);
}
