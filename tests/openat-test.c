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
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>

int main() {
  char * path;
  int fd = openat(AT_FDCWD, "test.txt", O_CREAT, 0666);
  close(fd);
  openat(AT_FDCWD, "test.txt", O_APPEND | O_RDWR);
  unlink("test.txt");
  fd = openat(-1, "test.txt", O_CREAT, 0666);
  close(fd);
  fd = openat(AT_FDCWD, path, 0);
  fd = openat(AT_FDCWD, "test.txt", 6625436, 24661);
  close(fd);
  return 0;
}
