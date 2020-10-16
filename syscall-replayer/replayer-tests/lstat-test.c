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

#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0600);
  struct stat *buf = (struct stat *) malloc(sizeof(struct stat));
  lstat("test.txt", buf);
  free(buf);
  buf = (struct stat *) malloc(sizeof(struct stat));
  lstat("nonexistent.txt", buf);
  free(buf);
  symlink("test.txt",
	  "test2.txt");
  buf = (struct stat *) malloc(sizeof(struct stat));
  lstat("test2.txt", buf);
  free(buf);
  unlink("test2.txt");
  link("test.txt",
       "test2.txt");
  buf = (struct stat *) malloc(sizeof(struct stat));
  lstat("test2.txt", buf);
  free(buf);
  unlink("test2.txt");
  mkdir("testtmp", 0700);
  buf = (struct stat *) malloc(sizeof(struct stat));
  lstat("testtmp", buf);
  free(buf);
  rmdir("testtmp");
  unlink("test.txt");
  return 0;
}
  
