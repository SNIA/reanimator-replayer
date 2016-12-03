/*
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * UmaskSystemCallTraceReplayModule header file
 *
 * Read UmaskSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "UmaskSystemCallTraceReplayModule.hpp"

UmaskSystemCallTraceReplayModule::
UmaskSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  mode_value_(series, "mode_value", Field::flag_nullable) {
  sys_call_name_ = "umask";
}

void UmaskSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("mode(", mode_value_.val(), ")");
}

void UmaskSystemCallTraceReplayModule::processRow() {
  // mode_t mode = mode_value_.val();
  // Replay umask by updating umask table.
}
