/*
 * Copyright (c) 2015 Leixiang Wu
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * A program simply read the input file.
 */
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
  if (argc < 3) {
    std::cerr << "Too few arguments!" << std::endl;
    std::cerr << "Usage: syscall-simple-read <filename>" << std::endl;
    return 1;
  }

  // open the input and output file
  int infd = open(argv[1], O_RDONLY);
  int outfd = open(argv[2], O_WRONLY);
  if (infd < -1){
    perror(argv[1]);
    return 1;
  }
  
  if (outfd < -1){
    perror(argv[2]);
    return 1;
  }
    
  int page_size = getpagesize();
  char buffer[page_size];
  
  int bytes_read = read(infd, buffer, page_size);
  
  int bytes_write = write(outfd, buffer, bytes_read);
  
  while(bytes_read > 0){
    std::cout << buffer;
    bytes_read = read(infd, buffer, page_size);
    bytes_write = write(outfd, buffer, bytes_read);
    if (bytes_write != bytes_read) {
      return 0;
    }
  }
  
  close(infd);
  close(outfd);
  return 0;
}

