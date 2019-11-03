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
#include <sys/mman.h>
#include "VirtualAddressSpace.hpp"

MmapSystemCallTraceReplayModule::MmapSystemCallTraceReplayModule(
    DataSeriesModule& source, bool verbose_flag, int warn_level_flag)
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
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, descriptorVal);
  void* replayed_addr;
  int64_t traced_addr = mmapReturnVal;

  replayed_addr = mmap(reinterpret_cast<void*>(startAddress), sizeOfMap,
                       protectionVal, flagsVal, fd, offsetVal);

  int64_t replayed_addr_int = reinterpret_cast<int64_t>(replayed_addr);
  if (startAddress != 0 && replayed_addr_int != traced_addr) return;

  // add the traced mmap to vm manager
  VM_manager* vm_manager = VM_manager::getInstance();
  VM_area* area = vm_manager->get_VM_area(pid);
  VM_node* node = new VM_node(reinterpret_cast<void*>(traced_addr),
                              replayed_addr, sizeOfMap, descriptorVal, fd);

  area->insert_VM_node(node);
  syscall_logger_->log_info(
      "pid(", pid,
      "), "
      "traced_address(",
      boost::format("%02x") % (uint64_t)traced_addr, "), ", "replayed_address(",
      boost::format("%02x") % replayed_addr, "), ", "length(", std::dec,
      sizeOfMap, "), ", "protection_value(", protectionVal, "), ",
      "flags_value(", flagsVal, "), ", "traced fd(", descriptorVal, "), ",
      "replayed_fd fd(", fd, "), ", "offset(",
      boost::format("%02x") % offsetVal, ")");

  area->list();
}

void MmapSystemCallTraceReplayModule::prepareRow() {
  startAddress = start_address_.val();
  sizeOfMap = length_.val();
  protectionVal = protection_value_.val();
  flagsVal = flags_value_.val();
  descriptorVal = descriptor_.val();
  offsetVal = offset_.val();
  mmapReturnVal = return_value_.val();
  SystemCallTraceReplayModule::prepareRow();
}
