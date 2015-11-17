/*
 * Copyright (c) 2015 Shubhi Rani
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * A program simply write patterns to an output file
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <fstream>

#define ITR 1000

int main(int argc, char *argv[]){
  if (argc < 2) {
    std::cerr << "Too few arguments!" << std::endl;
    std::cerr << "Usage: strace-write <filename>" << std::endl;
    return 1;
  }

  // open the output file
  int outfd = open(argv[1], O_WRONLY);

  if (outfd < -1){
    perror(argv[1]);
    return 1;
  }
 
  int i =0;
  time_t start, end;

  int page_size = getpagesize();
  char buffer[page_size];
  memset(buffer,'0',page_size);

  int bytes_write;

  /* To write zeros */
  start = time(NULL);
  while(i < ITR){
    bytes_write = write(outfd, buffer, page_size);
    i++;
  }
  end = time(NULL);
  std::cout << "Starting  Time:" << start << std::endl;
  std::cout << "Finishing Time:" << end << std::endl;
  std::cout << "Time taken in writing zeros:" << (end - start) << std::endl;

  #if 0
  /* To write fixed pattern */
  i =0;
  memset(buffer,'5',page_size);
  start = time(NULL);
  while(i < ITR){
    bytes_write = write(outfd, buffer, page_size);
    i++;
  }
  end = time(NULL);
  std::cout << "Starting  Time:" << start << std::endl;
  std::cout << "Finishing Time:" << end << std::endl;
  std::cout << "Time taken in writing fixed pattern:" << (end - start) << std::endl;

  /* To write random pattern */
  i=0;
  std::ifstream random_file_;
  random_file_.open("/dev/urandom");
  if (!random_file_.is_open()) {
    std::cerr << "Unable to open file '/dev/urandom/'.\n";
    exit(EXIT_FAILURE);
  }

  start = time(NULL);
  while(i < ITR){
    random_file_.read(buffer, page_size);
    bytes_write = write(outfd, buffer, page_size);
    i++;
  }
  end = time(NULL);
  std::cout << "Starting  Time:" << start << std::endl;
  std::cout << "Finishing Time:" << end << std::endl;
  std::cout << "Time taken in writing random pattern:" << (end - start) << std::endl;

  random_file_.close();
  #endif
  close(outfd);

  return 0;
}
