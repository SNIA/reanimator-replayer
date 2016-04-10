/*
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program creates a file, opens it and closes it.
 * Note: consult open man page for more information about
 *       open system call.
 *
 * USAGE:
 * ./open-test-prog
 */

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
  char tmp_name[] = "tmp-fileXXXXXX";
  int infd = mkstemp(tmp_name);
  if (infd < -1) {
    perror("open");
    return 1;
  }
  close(infd);
  if (unlink(tmp_name) != 0) {
    perror("unlink");
    return 1;
  }
  return 0;
}
