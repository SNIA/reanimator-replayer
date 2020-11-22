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
#include <linux/types.h>
#include <fcntl.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
  dev_t dev;

  dev = 0;
  mknod("test2.reg", 0666 | S_IFREG, dev);
  unlink("test2.reg");

  dev = makedev(16, 25);
  mknod("test2.chr", S_IFCHR, dev);
  unlink("test2.chr");
  
  mknod("test2.blk", S_IFBLK, dev);
  unlink("test2.blk");

  mknod("test2.fifo", 0600 | S_IFIFO, dev);
  unlink("test2.fifo");

  mknod("test2.sock", 0600 | S_IFSOCK, dev);
  unlink("test2.sock");

  mknod("test.txt", 0600 | S_IFREG, dev);
  unlink("test.txt");

  mknod("test2.txt", 5555555, dev);
  unlink("test2.txt");
  mknod("test2.blk", S_IFBLK, 0);
  unlink("test2.blk");

  return 0;
}
