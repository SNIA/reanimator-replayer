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
 * AcceptSystemCallTraceReplayModule header file
 *
 * Read AcceptSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "AcceptSystemCallTraceReplayModule.hpp"

AcceptSystemCallTraceReplayModule::AcceptSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      descriptor_value_(series, "descriptor") {
  sys_call_name_ = "accept";
}

void AcceptSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("socket fd(", socket_fd, ")");
}

void AcceptSystemCallTraceReplayModule::processRow() {
  const int traced_fd = return_value();

  if (traced_fd != SYSCALL_FAILURE) {
    /*
     * The traced accept() call was a success
     * Don't replay the accept system call.
     * Create a fake fd-map entry.
     */
    pid_t pid = executing_pid();
    replayer_resources_manager_.add_fd(pid, traced_fd, SYSCALL_SIMULATED, 0);
    replayed_ret_val_ = traced_fd;
  } else {
    // Traced accept() call was a failure. Replay returns failure.
    replayed_ret_val_ = -1;
  }
}

void AcceptSystemCallTraceReplayModule::prepareRow() {
  socket_fd = descriptor_value_.val();
  SystemCallTraceReplayModule::prepareRow();
}
