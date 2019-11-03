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
 * MunmapSystemCallTraceReplayModule header file.
 *
 * Read MunmapSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "MunmapSystemCallTraceReplayModule.hpp"
#include <sys/mman.h>
#include "VirtualAddressSpace.hpp"

MunmapSystemCallTraceReplayModule::MunmapSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      start_address_(series, "start_address"),
      length_(series, "length") {
  sys_call_name_ = "munmap";
}

void MunmapSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info(
      "start_address(", boost::format("0x%02x") % startAddress, "), ",
      "length(", boost::format("0x%02x") % sizeOfMap, ")");
}

void MunmapSystemCallTraceReplayModule::processRow() {
  VM_manager::getInstance()
      ->get_VM_area(executing_pid())
      ->delete_VM_node(reinterpret_cast<void *>(startAddress), sizeOfMap);
}

void MunmapSystemCallTraceReplayModule::prepareRow() {
  startAddress = start_address_.val();
  sizeOfMap = length_.val();
  SystemCallTraceReplayModule::prepareRow();
}
