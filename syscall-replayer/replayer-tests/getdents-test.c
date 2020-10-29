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
#include <sys/syscall.h>
#include <dirent.h>
#include <fcntl.h>

int main() {
  int fd = open(".", O_DIRECTORY);
  int count = 10000;
  struct linux_dirent *buf = malloc(count);
  syscall(SYS_getdents, fd, buf, count);
  free(buf);
  buf = malloc(count);
  syscall(SYS_getdents, -1, buf, count);
  free(buf);
  buf = malloc(count);
  syscall(SYS_getdents, fd, buf, 2);
  free(buf);
  buf = malloc(count);
  syscall(SYS_getdents, fd, buf, -1);
  free(buf);
  syscall(SYS_getdents, fd, NULL, count);
  close(fd);
  return 0;
}
