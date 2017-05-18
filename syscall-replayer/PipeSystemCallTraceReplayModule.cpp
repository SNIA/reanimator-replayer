/*
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
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

PipeSystemCallTraceReplayModule::
PipeSystemCallTraceReplayModule(DataSeriesModule &source,
                                bool verbose_flag,
                                bool verify_flag,
                                int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  verify_(verify_flag),
  read_descriptor_(series, "read_descriptor"),
  write_descriptor_(series, "write_descriptor") {
  sys_call_name_ = "pipe";
}

void PipeSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("read descriptor(", read_descriptor_.val(), "), ", \
    "write descriptor(", write_descriptor_.val(), ")");
}

void PipeSystemCallTraceReplayModule::processRow() {
  int pipefd[2];

  // replay the pipe system call
  if ((read_descriptor_.val() == 0) && (write_descriptor_.val() == 0)) {
    // If both descriptors were set as 0, pass NULL to the pipe call
    if (verify_) {
      syscall_logger_->log_info("Pipe was passed NULL instead of an integer array.");
    }
    replayed_ret_val_ = pipe(NULL);
  } else {
    replayed_ret_val_ = pipe(pipefd);
  }

  if (verify_) {
    /*
     * Verify that the file descriptors returned by pipe are the same as
     * in the trace.
     */
    if ((pipefd[0] != read_descriptor_.val()) ||
      (pipefd[1] != write_descriptor_.val())) {
      syscall_logger_->log_err("Captured and replayed pipe file descriptors differ.");
      if (verbose_mode()) {
        syscall_logger_->log_warn("Captured read descriptor: ",
          read_descriptor_.val(), ", Replayed read descriptor: ", pipefd[0]);
        syscall_logger_->log_warn("Captured write descriptor: ", \
          write_descriptor_.val(), ", Replayed write descriptor: ", pipefd[1]);
      }
      if (abort_mode())
        abort();
    }
  }
  /*
   * Add mappings from the recorded file descriptors to the
   * replayed file descriptors
   */
  pid_t pid = executing_pid();
  /*
   * If flags is 0, then pipe2() is the same as pipe(). Therefore, fds created by
   * pipe have flags 0.
   */
  replayer_resources_manager_.add_fd(pid, read_descriptor_.val(), pipefd[0], 0);
  replayer_resources_manager_.add_fd(pid, write_descriptor_.val(), pipefd[1], 0);
}
