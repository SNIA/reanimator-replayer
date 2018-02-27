/*
 * Copyright (c) 2016-2016 Nina Brown
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the DataSeriesOutputModule.hpp
 * header file.
 *
 * Read the DataSeriesOutputModule.hpp file for more information about this
 * class.
 */

#include "DataSeriesOutputModule.hpp"

void DataSeriesOutputModule::makePipeArgsMap(void **args_map,
					     long *args,
					     void **v_args) {
  static int pipefd[2];

  if (v_args[0] != NULL) {
    pipefd[0] = ((int *) v_args[0])[0];
    pipefd[1] = ((int *) v_args[0])[1];
  } else {
    /*
     * In the case of a NULL file descriptor array, set
     * read_descriptor and write_descriptor equal to 0.
     */
    pipefd[0] = 0;
    pipefd[1] = 0;
    std::cerr << "Pipe: File descriptor array is set as NULL!!" << std::endl;
  }

  args_map[SYSCALL_FIELD_READ_DESCRIPTOR] = &pipefd[0];
  args_map[SYSCALL_FIELD_WRITE_DESCRIPTOR] = &pipefd[1];
}
