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
 * MmapSystemCallTraceReplayModule header file.
 *
 * Read MmapSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "MmapSystemCallTraceReplayModule.hpp"

MmapSystemCallTraceReplayModule::MmapSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      start_address_(series, "start_address"),
      length_(series, "length"),
      protection_value_(series, "protection_value", Field::flag_nullable),
      flags_value_(series, "flags_value", Field::flag_nullable),
      descriptor_(series, "descriptor"),
      offset_(series, "offset") {
  sys_call_name_ = "mmap";
}

void MmapSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptorVal);

  syscall_logger_->log_info(
      "start_address(", boost::format("0x%02x") % startAddress, "), ",
      "length(", std::dec, sizeOfMap, "), ", "protection_value(", protectionVal,
      "), ", "flags_value(", flagsVal, "), ", "traced fd(", descriptorVal,
      "), ", "replayed_fd fd(", replayed_fd, "), ", "offset(",
      boost::format("0x%02x") % offsetVal, ")");
}

void MmapSystemCallTraceReplayModule::processRow() {
  /*
   * NOTE: It is inappropriate to replay mmap system call.
   * Hence we do not replay mmap system call.
   */
  return;
}

void MmapSystemCallTraceReplayModule::prepareRow() {
  if (verbose_) {
    startAddress = start_address_.val();
    sizeOfMap = length_.val();
    protectionVal = protection_value_.val();
    flagsVal = flags_value_.val();
    descriptorVal = descriptor_.val();
    offsetVal = offset_.val();
  }
  SystemCallTraceReplayModule::prepareRow();
}
