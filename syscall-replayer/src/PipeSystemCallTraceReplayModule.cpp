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
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * PipeSystemCallTraceReplayModule header file
 *
 * Read PipeSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "PipeSystemCallTraceReplayModule.hpp"

PipeSystemCallTraceReplayModule::PipeSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      verify_(verify_flag),
      read_descriptor_(series, "read_descriptor"),
      write_descriptor_(series, "write_descriptor") {
  sys_call_name_ = "pipe";
}

void PipeSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("read descriptor(", read_fd, "), ",
                            "write descriptor(", write_fd, ")");
}

void PipeSystemCallTraceReplayModule::processRow() {
  int pipefd[2];

  // replay the pipe system call
  if ((read_fd == 0) && (write_fd == 0)) {
    // If both descriptors were set as 0, pass NULL to the pipe call
    if (verify_) {
      syscall_logger_->log_info(
          "Pipe was passed NULL instead of an integer array.");
    }
    replayed_ret_val_ = pipe(nullptr);
  } else {
    replayed_ret_val_ = pipe(pipefd);
  }

  if (verify_) {
    /*
     * Verify that the file descriptors returned by pipe are the same as
     * in the trace.
     */
    if ((pipefd[0] != read_fd) || (pipefd[1] != write_fd)) {
      syscall_logger_->log_err(
          "Captured and replayed pipe file descriptors differ.");
      if (verbose_mode()) {
        syscall_logger_->log_warn("Captured read descriptor: ", read_fd,
                                  ", Replayed read descriptor: ", pipefd[0]);
        syscall_logger_->log_warn("Captured write descriptor: ", write_fd,
                                  ", Replayed write descriptor: ", pipefd[1]);
      }
      if (abort_mode()) {
        abort();
      }
    }
  }
  /*
   * Add mappings from the recorded file descriptors to the
   * replayed file descriptors
   */
  pid_t pid = executing_pid();
  /*
   * If flags is 0, then pipe2() is the same as pipe(). Therefore, fds created
   * by
   * pipe have flags 0.
   */
  replayer_resources_manager_.add_fd(pid, read_fd, SYSCALL_SIMULATED, 0);
  replayer_resources_manager_.add_fd(pid, write_fd, SYSCALL_SIMULATED, 0);
}

void PipeSystemCallTraceReplayModule::prepareRow() {
  read_fd = read_descriptor_.val();
  write_fd = write_descriptor_.val();
  SystemCallTraceReplayModule::prepareRow();
}
