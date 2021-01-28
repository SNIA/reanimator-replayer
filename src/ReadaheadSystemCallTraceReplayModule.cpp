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
 * ReadaheadSystemCallTraceReplayModule header file.
 *
 * Read ReadaheadSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "ReadaheadSystemCallTraceReplayModule.hpp"

ReadaheadSystemCallTraceReplayModule::ReadaheadSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      descriptor_(series, "descriptor"),
      readahead_off_(series, "readahead_off"),
      readahead_size_(series, "readahead_size") {
  sys_call_name_ = "readahead";
}

void ReadaheadSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, traced_fd);
  syscall_logger_->log_info("traced fd(", traced_fd, "), ", "replayed fd(",
                            replayed_fd, "), ", "offset(", offset, "), ",
                            "size(", size, ")");
}

void ReadaheadSystemCallTraceReplayModule::processRow() {
  // Get actual file descriptor
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, traced_fd);

  if (fd == SYSCALL_SIMULATED) {
    /*
     * FD for the readahead call originated from a socket().
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = simulated_ret_val;
    return;
  }

  replayed_ret_val_ = readahead(fd, offset, size);
}

void ReadaheadSystemCallTraceReplayModule::prepareRow() {
  simulated_ret_val = return_value_.val();
  traced_fd = descriptor_.val();
  offset = readahead_off_.val();
  size = readahead_size_.val();
  SystemCallTraceReplayModule::prepareRow();
}
