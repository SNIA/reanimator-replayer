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
 * SocketPairSystemCallTraceReplayModule header file
 *
 * Read SocketPairSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "SocketPairSystemCallTraceReplayModule.hpp"

SocketPairSystemCallTraceReplayModule::SocketPairSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      read_descriptor_(series, "read_descriptor"),
      write_descriptor_(series, "write_descriptor") {
  sys_call_name_ = "socketpair";
}

void SocketPairSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("read descriptor(", read_fd, "), ",
                            "write descriptor(", write_fd, ")");
}

void SocketPairSystemCallTraceReplayModule::processRow() {
  pid_t pid = executing_pid();
  replayer_resources_manager_.add_fd(pid, read_fd, SYSCALL_SIMULATED, 0);
  replayer_resources_manager_.add_fd(pid, write_fd, SYSCALL_SIMULATED, 0);
}

void SocketPairSystemCallTraceReplayModule::prepareRow() {
  read_fd = read_descriptor_.val();
  write_fd = write_descriptor_.val();
  SystemCallTraceReplayModule::prepareRow();
}
