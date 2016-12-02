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
 * ExitSystemCallTraceReplayModule header file.
 *
 * Read ExitSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "ExitSystemCallTraceReplayModule.hpp"

ExitSystemCallTraceReplayModule::
ExitSystemCallTraceReplayModule(DataSeriesModule &source,
				bool verbose_flag,
				int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  exit_status_(series, "exit_status"),
  generated_(series, "generated") {
  sys_call_name_ = "exit";
}

void ExitSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("exit_status(", exit_status_.val(), "), ", \
    "generated(", generated_.val(), ")");
}

void ExitSystemCallTraceReplayModule::processRow() {
  /*
   * NOTE: On replaying exit system call, our replayer will terminate.
   * Hence we do not replay exit system call, but we update replayer resources
   */
  pid_t pid = executing_pid();
  // Clone umask table
  SystemCallTraceReplayModule::replayer_resources_manager_.remove_umask(pid);
  return;
}
