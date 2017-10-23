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
 * SocketSystemCallTraceReplayModule header file
 *
 * Read SocketSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "SocketSystemCallTraceReplayModule.hpp"

SocketSystemCallTraceReplayModule::
SocketSystemCallTraceReplayModule(DataSeriesModule &source,
  bool verbose_flag,
  int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  domain_value_(series, "domain"),
  type_value_(series, "type"),
  protocol_value_(series, "protocol") {
  sys_call_name_ = "socket";
}

void SocketSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("domain(", domain_value_.val(),
    "), type(", type_value_.val(),
    "), protocol(", protocol_value_.val(), ")");
}

void SocketSystemCallTraceReplayModule::processRow() {
  const int traced_fd = static_cast<int>(return_value_.val());

  if (traced_fd != SYSCALL_FAILURE) {
    /*
     * The traced socket() call was a success
     * Don't replay the socket system call.
     * Create a fake fd-map entry.
     */
    pid_t pid = executing_pid();
    replayer_resources_manager_.add_fd(pid, traced_fd,
				       SYSCALL_SIMULATED, 0); // fake FD
    replayed_ret_val_ = traced_fd;
  } else {
    // Traced socket() call was a failure. Replay returns failure.
    replayed_ret_val_ = -1;
  }
}
