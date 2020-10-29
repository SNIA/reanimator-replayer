/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
 */

#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define TEST_BUF_SIZE 4096
// #define MAX_READS 2 * 1024 * 1024
#define MAX_READS 64 * 1024
#define FILE_NAME "myfile"

uint64_t getRandom(uint64_t lower, uint64_t upper) {
  return (rand() % (upper - lower + 1)) + lower;
}

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
    uint64_t upperBound = 8 * 1024 * 1024;
    upperBound *= 1024;
    uint64_t random = getRandom(0, upperBound);
    random = (random/TEST_BUF_SIZE)*TEST_BUF_SIZE;
    if (lseek(fd, random, SEEK_SET) < 0) {
      perror("lseek");
      exit(3);
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
  srand(time(0));

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
