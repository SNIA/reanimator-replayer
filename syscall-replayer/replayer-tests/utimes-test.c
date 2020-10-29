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

#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

int main() {
  struct timeval acc;
  struct timeval mod;
  struct timeval tv[2];
  const char * notpath;

  int fd = open("test.txt", O_CREAT, 0600);

  gettimeofday(&acc, NULL);
  struct timespec sleep;
  sleep.tv_sec = 0;
  sleep.tv_nsec = 500;
  nanosleep(&sleep, NULL);
  gettimeofday(&mod, NULL);

  tv[0] = acc;
  tv[1] = mod;
  utimes("test.txt", tv);

  nanosleep(&sleep, NULL);

  utimes(notpath, tv);
  utimes("nonexistent.txt", tv);
  utimes("test.txt", NULL);

  gettimeofday(&acc, NULL);
  gettimeofday(&mod, NULL);
  tv[0] = acc;
  tv[1] = mod;
  utimes("test.txt", tv);

  close(fd);
  unlink("test.txt");
  return 0;
}
