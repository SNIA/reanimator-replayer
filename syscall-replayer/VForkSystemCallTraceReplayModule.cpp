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
 * VForkSystemCallTraceReplayModule header file.
 *
 * Read VForkSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "VForkSystemCallTraceReplayModule.hpp"

VForkSystemCallTraceReplayModule::
VForkSystemCallTraceReplayModule(DataSeriesModule &source,
                                 bool verbose_flag,
                                 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag) {
  sys_call_name_ = "vfork";
}

void VForkSystemCallTraceReplayModule::print_specific_fields() {
}

void VForkSystemCallTraceReplayModule::processRow() {
  /*
   * Here, we will create a new file descriptor mapping for
   * the process created by vfork.
   * NOTE: It is inappropriate to replay vfork system call.
   * Hence we do not replay vfork system call.
   */
  /* A call to vfork() is equivalent to calling clone(2) with flags
   * specified as: CLONE_VM | CLONE_VFORK | SIGCHLD
   */
  bool shared_umask = false, shared_files = false;
  pid_t ppid = executing_pid();
  pid_t pid = return_value();
  // Clone resources tables
  SystemCallTraceReplayModule::replayer_resources_manager_.clone_umask(ppid, pid, shared_umask);
  SystemCallTraceReplayModule::replayer_resources_manager_.clone_fd_table(ppid, pid, shared_files);
}
