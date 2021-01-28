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

#include <utime.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
  struct utimbuf times;

  int fd = open("test.txt", O_CREAT, 0600);
  utime("test.txt", &times);
  times.actime = time(NULL);
  times.modtime = time(NULL);
  utime("test.txt", &times);
  utime ("nonexistent.txt", &times);
  utime("test.txt", NULL);
  struct timespec sleeptime;
  sleeptime.tv_sec = 2;
  sleeptime.tv_nsec = 0;
  nanosleep(&sleeptime, NULL);
  times.actime = time(NULL);
  times.modtime = time(NULL);
  utime("test.txt", &times);

  close(fd);
  unlink("test.txt");
  return 0;
}
