/*
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * ChrootSystemCallTraceReplayModule header file
 *
 * Read ChrootSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "ChrootSystemCallTraceReplayModule.hpp"

ChrootSystemCallTraceReplayModule::ChrootSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      given_pathname_(series, "given_pathname", Field::flag_nullable) {
  sys_call_name_ = "chroot";
}

void ChrootSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", pathname, ")");
}

void ChrootSystemCallTraceReplayModule::processRow() {
  replayed_ret_val_ = chroot(pathname);
  delete[] pathname;
}

void ChrootSystemCallTraceReplayModule::prepareRow() {
  auto pathBuf = reinterpret_cast<const char *>(given_pathname_.val());
  pathname = new char[std::strlen(pathBuf) + 1];
  std::strncpy(pathname, pathBuf, std::strlen(pathBuf) + 1);
  SystemCallTraceReplayModule::prepareRow();
}
