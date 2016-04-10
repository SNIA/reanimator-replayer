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
 * This file implements all the functions in the LinkSystemCallTraceReplayModule
 * header file
 *
 * Read LinkSystemCallTraceReplayModule.hpp for more information about this class.
 */

#include "LinkSystemCallTraceReplayModule.hpp"

LinkSystemCallTraceReplayModule::LinkSystemCallTraceReplayModule(DataSeriesModule &source,
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

void LinkSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Link System Call Replayer starts to replay...-----" << std::endl;
}

void LinkSystemCallTraceReplayModule::processRow() {
  const char *old_path_name = (const char *)given_oldpathname_.val();
  const char *new_path_name = (const char *)given_newpathname_.val();

  // Replay the link system call
  replayed_ret_val_ = link(old_path_name, new_path_name);
}

void LinkSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Link System Call Replayer finished replaying...-----" << std::endl;
}
