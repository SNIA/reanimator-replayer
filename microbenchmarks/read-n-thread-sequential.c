#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TEST_BUF_SIZE 4096
// #define MAX_READS 2 * 1024 * 1024
#define MAX_READS 1024 * 1024
#define FILE_NAME "myfile"

void *threadFunc(void *arg) {
  char buf[TEST_BUF_SIZE];
  char threadIdBuf[15];
  int fd, i, numbytes;
  uint64_t threadId = (uint64_t)arg;

  snprintf(threadIdBuf, sizeof(threadIdBuf), "%s%d", FILE_NAME, threadId);
  printf("%s\n", threadIdBuf);
  fd = open(threadIdBuf, O_RDONLY);
  if (fd < 0) {
    perror("open " FILE_NAME " error");
    exit(1);
  }

  for (i = 0; i < MAX_READS; i++) {
    numbytes = read(fd, buf, TEST_BUF_SIZE);
    if (numbytes < 0) {
      perror("read");
      exit(2);
    }
    if (numbytes < TEST_BUF_SIZE) {
      fprintf(stderr, "Seek count=%d\n", i);
      if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek");
        exit(3);
      }
    }
  }

  close(fd);
}

int main(int argc, char *argv[]) {
  char c;
  uint64_t nthreads = 0;

  while ((c = getopt(argc, argv, "n:")) != -1) {
    switch (c) {
    case 'n': {
      nthreads = atoi(optarg);
      break;
    }
    default:
      break;
    }
  }
  if (nthreads == 0) {
    nthreads = 1;
  }
  // printf("number of threads %d\n", nthreads);
  if (nthreads == 1) {
    threadFunc((void *)nthreads);
  } else {
    pthread_t *array = malloc(sizeof(pthread_t) * nthreads);
    for (uint64_t i = 0; i < nthreads; i++) {
      pthread_create(&array[i], NULL, threadFunc, (void *)(i + 1));
    }
    for (uint64_t i = 0; i < nthreads; i++) {
      pthread_join(array[i], NULL);
    }
  }

  exit(0);
}
