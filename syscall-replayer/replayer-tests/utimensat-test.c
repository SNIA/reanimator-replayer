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
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

int main() {
  struct timespec ts[2];
  const char *path = NULL;
  int fd = creat("test.txt", 0666);
  ts[0].tv_sec = 500;
  ts[0].tv_nsec = 500;
  ts[1].tv_sec = 400;
  ts[1].tv_nsec = 400;
  syscall(SYS_utimensat, fd, NULL, ts, 0);
  const char *path1 = "test.txt";
  utimensat(AT_FDCWD, path1, ts, AT_SYMLINK_NOFOLLOW);
  utimensat(AT_FDCWD, path1, NULL, 0);
  utimensat(fd, "", ts, 0);
  unlink("test.txt");
  return 0;
}
