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

  extern char *optarg;
  int flag=0, iterations=0;
  while(1){
                int c ;
                c= getopt(argc, argv, ":n:h");
                if (c == -1)
                        break;
                switch(c){
                        case 'n':
                                iterations = atoi(optarg);
                                flag +=1;
                                break;
			case 'h':
                                std::cout << "Use -n option to provide #iterations" << std::endl;
                                break;
   			case '?':
                                std::cerr << "Unknown option character" << optopt << std::endl;
                                return 1;
                        default:
                                break;
                }
        }

  if(flag == 0){
                std::cout << "Please specify number of ITERATIONS" << std::endl;
                exit(2);
        }

  if(flag > 1){
                std::cout << "-n option is given twice" << std::endl;
                exit(2);
        }

  if (argc < 4) {
    std::cerr << "Too few arguments!" << std::endl;
    std::cerr << "Usage: write-simple-program <-n #iterations> <filename>" << std::endl;
    return 1;
  }

  // open the output file
  int outfd = open(argv[3], O_WRONLY);

  if (outfd < -1){
    perror(argv[1]);
    return 1;
  }
 
  int i =0;
  time_t start, end;

  int page_size = getpagesize();
  char buffer[page_size];
  memset(buffer,'5',page_size);

  int bytes_write =0;

    
  /* To write zeros */
  start = time(NULL);
  while(i < iterations){
    bytes_write = write(outfd, buffer, page_size);
    i++;
    if(bytes_write < 0)
	std::cout << "Write Error!" << std::endl;
  }
  
  end = time(NULL);
  std::cout << "Starting  Time:" << start << std::endl;
  std::cout << "Finishing Time:" << end << std::endl;
  std::cout << "Time taken in writing zeros:" << (end - start) << std::endl;

  close(outfd);

  return 0;
}
