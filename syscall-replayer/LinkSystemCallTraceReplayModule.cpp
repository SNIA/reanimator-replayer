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
 * LinkSystemCallTraceReplayModule header file
 *
 * Read LinkSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "LinkSystemCallTraceReplayModule.hpp"

LinkSystemCallTraceReplayModule::
LinkSystemCallTraceReplayModule(DataSeriesModule &source,
				bool verbose_flag,
				int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_oldpathname_(series, "given_oldpathname"),
  given_newpathname_(series, "given_newpathname") {
  sys_call_name_ = "link";
}

void LinkSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "old path(" << given_oldpathname_.val() << "), ";
  std::cout << "new path(" << given_newpathname_.val() << ")";
}

void LinkSystemCallTraceReplayModule::processRow() {
  const char *old_path_name = (const char *)given_oldpathname_.val();
  const char *new_path_name = (const char *)given_newpathname_.val();

  // Replay the link system call
  replayed_ret_val_ = link(old_path_name, new_path_name);
}

LinkatSystemCallTraceReplayModule::
LinkatSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  int warn_level_flag):
  LinkSystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  old_descriptor_(series, "old_descriptor"),
  new_descriptor_(series, "new_descriptor"),
  flag_value_(series, "flag_value", Field::flag_nullable) {
  sys_call_name_ = "linkat";
}

void LinkatSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "old descriptor(" << old_descriptor_.val() << "), ";
  std::cout << "new descriptor(" << new_descriptor_.val() << "), ";
  LinkSystemCallTraceReplayModule::print_specific_fields();
  std::cout << ", ";
  std::cout << "flags(" << flag_value_.val() << ")";
}

void LinkatSystemCallTraceReplayModule::processRow() {
  int old_fd = SystemCallTraceReplayModule::fd_map_[old_descriptor_.val()];
  int new_fd = SystemCallTraceReplayModule::fd_map_[new_descriptor_.val()];
  const char *old_path_name = (const char *)given_oldpathname_.val();
  const char *new_path_name = (const char *)given_newpathname_.val();
  int flags = flag_value_.val();

  // Replay the linkat system call
  replayed_ret_val_ = linkat(old_fd, old_path_name,
			     new_fd, new_path_name,
			     flags);
}
