/*
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * ChmodSystemCallTraceReplayModule header file
 *
 * Read ChmodSystemCallTraceReplayModule.hpp for more information about this
 * class.
 */

#include "ChmodSystemCallTraceReplayModule.hpp"

ChmodSystemCallTraceReplayModule::ChmodSystemCallTraceReplayModule(
					   DataSeriesModule &source,
					   bool verbose_flag,
					   int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_pathname_(series, "given_pathname"),
  mode_value_(series, "mode_value", Field::flag_nullable) {
  sys_call_name_ = "chmod";
}

void ChmodSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", given_pathname_.val(), "), ", \
    "traced mode(", mode_value_.val(), "), ",
    "replayed mode(", get_mode(mode_value_.val()), ")");
}

void ChmodSystemCallTraceReplayModule::processRow() {
  const char *pathname = (char *)given_pathname_.val();
  mode_t mode = get_mode(mode_value_.val());

  // Replay the chmod system call
  replayed_ret_val_ = chmod(pathname, mode);
}
