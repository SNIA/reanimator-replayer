/*
 * Copyright (c) 2016 Nina Brown
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
 * UnlinkSystemCallTraceReplayModule header file
 *
 * Read UnlinkSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "UnlinkSystemCallTraceReplayModule.hpp"

UnlinkSystemCallTraceReplayModule::
UnlinkSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_pathname_(series, "given_pathname") {
  sys_call_name_ = "unlink";
}

void UnlinkSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "pathname(" << given_pathname_.val() << ")";
}

void UnlinkSystemCallTraceReplayModule::processRow() {
  char *path = (char *) given_pathname_.val();
  // Replay the unlink system call
  replayed_ret_val_ = unlink(path);
}

UnlinkatSystemCallTraceReplayModule::
UnlinkatSystemCallTraceReplayModule(DataSeriesModule &source,
				    bool verbose_flag,
				    int warn_level_flag):
  UnlinkSystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  descriptor_(series, "descriptor"),
  flag_value_(series, "flag_value", Field::flag_nullable) {
  sys_call_name_ = "unlinkat";
}

void UnlinkatSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "descriptor(" << descriptor_.val() << "), ";
  UnlinkSystemCallTraceReplayModule::print_specific_fields();
  std::cout << ", ";
  std::cout << "flags(" << flag_value_.val() << ")";
}

void UnlinkatSystemCallTraceReplayModule::processRow() {
  int dirfd;
  char *path = (char *) given_pathname_.val();
  int flags = flag_value_.val();

  if (descriptor_.val() == AT_FDCWD) {
    dirfd = descriptor_.val();
  } else {
    dirfd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  }

  // Replay the unlinkat system call
  replayed_ret_val_ = unlinkat(dirfd, path, flags);
}
