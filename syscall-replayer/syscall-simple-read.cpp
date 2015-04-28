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
  if (argc < 2) {
    std::cerr << "Too few arguments!" << std::endl;
    std::cerr << "Usage: syscall-simple-read <filename>" << std::endl;
    return 1;
  }

  // open the input file
  int fd = open(argv[1], O_RDONLY);
  if (fd < -1){
    perror(argv[1]);
    return 1;
  }

  int page_size = getpagesize();
  char buffer[page_size];
	
  int bytes_read = read(fd, buffer, page_size);

  while(bytes_read > 0){
    std::cout << buffer;
    bytes_read = read(fd, buffer, page_size);
  }

  close(fd);
  return 0;
}

