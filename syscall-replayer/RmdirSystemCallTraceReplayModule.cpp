/*
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * RmdirSystemCallTraceReplayModule header file.
 *
 * Read RmdirSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "RmdirSystemCallTraceReplayModule.hpp"

RmdirSystemCallTraceReplayModule::
RmdirSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_pathname_(series, "given_pathname") {
  sys_call_name_ = "rmdir";
}

void RmdirSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("given_pathname(", pathname, ")");
}

void RmdirSystemCallTraceReplayModule::processRow() {
  // Replay rmdir sys call.
  replayed_ret_val_ = rmdir(pathname);
  delete[] pathname;
}

void RmdirSystemCallTraceReplayModule::prepareRow() {
  auto pathBuf = reinterpret_cast<const char *>(given_pathname_.val());
  pathname = new char[std::strlen(pathBuf)+1];
  std::strcpy(pathname, pathBuf);
  SystemCallTraceReplayModule::prepareRow();
}
