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
 * MkdirSystemCallTraceReplayModule header file
 *
 * Read MkdirSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "MkdirSystemCallTraceReplayModule.hpp"

MkdirSystemCallTraceReplayModule::
MkdirSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_pathname_(series, "given_pathname"),
  mode_value_(series, "mode_value", Field::flag_nullable) {
  sys_call_name_ = "mkdir";
}

void MkdirSystemCallTraceReplayModule::print_specific_fields() {
  LOG_INFO("pathname(" << given_pathname_.val() << "), " \
	   << "mode(" << mode_value_.val() << ")");
}

void MkdirSystemCallTraceReplayModule::processRow() {
  const char *pathname = (char *)given_pathname_.val();
  mode_t mode = mode_value_.val();

  // Replay the mkdir system call
  replayed_ret_val_ = mkdir(pathname, mode);
}

MkdiratSystemCallTraceReplayModule::
MkdiratSystemCallTraceReplayModule(DataSeriesModule &source,
				   bool verbose_flag,
				   int warn_level_flag):
  MkdirSystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  descriptor_(series, "descriptor") {
  sys_call_name_ = "mkdirat";
}

void MkdiratSystemCallTraceReplayModule::print_specific_fields() {
  LOG_INFO("descriptor(" << descriptor_.val() << "), " \
    << "pathname(" << given_pathname_.val() << "), " \
    << "mode(" << mode_value_.val() << ")");
}

void MkdiratSystemCallTraceReplayModule::processRow() {
  int dirfd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  const char *pathname = (char *)given_pathname_.val();
  mode_t mode = mode_value_.val();

  // Replay the mkdirat system call
  replayed_ret_val_ = mkdirat(dirfd, pathname, mode);
}
