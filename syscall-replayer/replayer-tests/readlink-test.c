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
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0600);

  symlink("test.txt", "test2.txt");

  char *buf = (char *)malloc(10);
  readlink("test2.txt", buf, 10);
  free(buf);

  buf = (char *)malloc(3);
  readlink("test2.txt", buf, 3);
  free(buf);

  buf = (char *)malloc(10);
  readlink("nonexistent.txt", buf, 10);
  free(buf);

  mkdir("testtmp", 0600);

  buf = (char *)malloc(10);
  readlink("testtmp", buf, 10);
  free(buf);

  rmdir("testtmp");

  symlink("test2.txt", "test3.txt");

  buf = (char *)malloc(10);
  readlink("test3.txt", buf, 10);
  free(buf);

  unlink("test2.txt");

  buf = (char *)malloc(10);
  readlink("test3.txt", buf, 10);
  free(buf);

  link("test.txt", "test2.txt");

  buf = (char *)malloc(10);
  readlink("test2.txt", buf, 10);
  free(buf);

  unlink("test3.txt");
  unlink("test2.txt");
  close(fd);
  unlink("test.txt");
  return 0;
}
