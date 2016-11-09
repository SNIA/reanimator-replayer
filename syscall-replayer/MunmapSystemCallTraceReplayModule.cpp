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
 * MunmapSystemCallTraceReplayModule header file.
 *
 * Read MunmapSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "MunmapSystemCallTraceReplayModule.hpp"

MunmapSystemCallTraceReplayModule::
MunmapSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  start_address_(series, "start_address"),
  length_(series, "length") {
  sys_call_name_ = "munmap";
}

void MunmapSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("start_address(", \
    int2base(start_address_.val(), std::hex), "), ", \
    "length(", length_.val(), ")");
}

void MunmapSystemCallTraceReplayModule::processRow() {
  /*
   * NOTE: It is inappropriate to replay munmap system call.
   * Hence we do not replay munmap system call.
   */
  return;
}
