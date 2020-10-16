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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define TEST_BUF_SIZE	128
#define MAX_READS 1000000 /* number of times to read the file */
#define FILE_NAME	"my_test.random"

int main(){

  char buf[TEST_BUF_SIZE];
  int fd, i, numbytes;

  fd = open(FILE_NAME, O_RDONLY);
  if (fd < 0) {
    perror("open " FILE_NAME " error");
    exit(1);
  }

  /* read 1M till end of file, then seek to start */

  for (i = 0; i < MAX_READS; i++) {
    numbytes = read(fd, buf, TEST_BUF_SIZE);
    if (numbytes < 0) {
      perror("read");
      exit(2);
    }
    if (numbytes < TEST_BUF_SIZE) { /* partial read or EOF, seek to start */
      fprintf(stderr, "Seek count=%d\n", i);
      if (lseek(fd, 0, SEEK_SET) < 0) {
	perror("lseek");
	exit(3);
      }
    }
  }
  close(fd);
  exit(0);
}
