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
  new_tls_(series, "new_tls", Field::flag_nullable) {
  sys_call_name_ = "clone";
}

void CloneSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("flags(", \
	   format_field_value(flag_value_.val(), std::hex), "), ",  \
	   "child stack address(", child_stack_address_.val(), "), ", \
	   "parent thread id(", parent_thread_id_.val(), "), ", \
	   "child thread id(", child_thread_id_.val(), "), ", \
	   "new tls(", new_tls_.val(), ")");
}

void CloneSystemCallTraceReplayModule::processRow() {
  /*
   * Here, we will create a new file descriptor mapping for
   * the process created by clone. If the CLONE_FILES flag is not set,
   * then the cloned process will get its own file descriptor map copied
   * from that of the parent process. If that flag is set, then the cloned
   * process id will be mapped to the parent process id, and the two processes 
   * will share a file descriptor table, as they would in the kernel.
   * NOTE: It is inappropriate to replay clone system call.
   * Hence we do not replay clone system call.
   */
  int flags = flag_value_.val();
  bool shared_umask = false;
  if (flags & CLONE_FS) {
    shared_umask = true;
  }
  pid_t ppid = executing_pid();
  pid_t pid = return_value();
  // Clone umask table
  SystemCallTraceReplayModule::replayer_resources_manager_.clone_umask(ppid, pid, shared_umask);
  return;
}
