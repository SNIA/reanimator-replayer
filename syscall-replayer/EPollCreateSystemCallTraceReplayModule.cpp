/*
 * Copyright (c) 2017 Darshan Godhia
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * EPollCreateSystemCallTraceReplayModule header file
 *
 * Read EPollCreateSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "EPollCreateSystemCallTraceReplayModule.hpp"

EPollCreateSystemCallTraceReplayModule::EPollCreateSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      size_value_(series, "epoll_size") {
  sys_call_name_ = "epoll_create";
}

void EPollCreateSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("size(", size, ")");
}

void EPollCreateSystemCallTraceReplayModule::processRow() {
  const int traced_fd = return_value();

  if (traced_fd != SYSCALL_FAILURE) {
    /*
     * The traced epoll_create() call was a success
     * Don't replay the epoll_create system call.
     * Create a fake fd-map entry.
     */
    pid_t pid = executing_pid();
    replayer_resources_manager_.add_fd(pid, traced_fd, SYSCALL_SIMULATED, 0);
    replayed_ret_val_ = traced_fd;
  } else {
    // Traced epoll_create() call was a failure. Replay returns failure.
    replayed_ret_val_ = -1;
  }
}

void EPollCreateSystemCallTraceReplayModule::prepareRow() {
  size = size_value_.val();
  SystemCallTraceReplayModule::prepareRow();
}
