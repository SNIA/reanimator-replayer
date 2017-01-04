/*
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * CreatSystemCallTraceReplayModule header file
 *
 * Read CreatSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "CreatSystemCallTraceReplayModule.hpp"

CreatSystemCallTraceReplayModule::
CreatSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_pathname_(series, "given_pathname"),
  mode_value_(series, "mode_value", Field::flag_nullable) {
  sys_call_name_ = "creat";
}

void CreatSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", given_pathname_.val(), "), ", \
    "traced mode(", mode_value_.val(), "), ",
    "replayed mode(", get_mode(mode_value_.val()), ")");
}

void CreatSystemCallTraceReplayModule::processRow() {
  const char *pathname = (char *)given_pathname_.val();
  mode_t mode = get_mode(mode_value_.val());
  int return_value = (int)return_value_.val();

  // replay the creat system call
  replayed_ret_val_ = creat(pathname, mode);
  // Add a mapping from fd in trace file to actual replayed fd
  pid_t pid = executing_pid();
  // A call to creat() is equivalent to calling open() with flags equal to
  // O_CREAT|O_WRONLY|O_TRUNC.
  replayer_resources_manager_.add_fd(pid, return_value, replayed_ret_val_, O_CREAT|O_WRONLY|O_TRUNC);
}
