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
 * CloneSystemCallTraceReplayModule header file.
 *
 * Read CloneSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "CloneSystemCallTraceReplayModule.hpp"

CloneSystemCallTraceReplayModule::
CloneSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  flag_value_(series, "flag_value", Field::flag_nullable),
  child_stack_address_(series, "child_stack_address"),
  parent_thread_id_(series, "parent_thread_id", Field::flag_nullable),
  child_thread_id_(series, "child_thread_id", Field::flag_nullable),
  pt_regs_(series, "pt_regs", Field::flag_nullable) {
  sys_call_name_ = "clone";
}

void CloneSystemCallTraceReplayModule::print_specific_fields() {
  LOG_INFO("flags(" << std::hex << flag_value_.val() << "), " \
	   << "child stack address(" << child_stack_address_.val() << "), " \
	   << "parent thread id(" << parent_thread_id_.val() << "), " \
	   << "child thread id(" << child_thread_id_.val() << ")");
}

void CloneSystemCallTraceReplayModule::processRow() {
  // Create new file descriptor mapping

  /*
   * NOTE: It is inappropriate to replay clone system call.
   * Hence we do not replay clone system call.
   */
  return;
}
