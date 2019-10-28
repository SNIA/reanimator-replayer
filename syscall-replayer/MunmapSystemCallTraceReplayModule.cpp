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
#include "VirtualAddressSpace.hpp"
#include <sys/mman.h>

MunmapSystemCallTraceReplayModule::MunmapSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      start_address_(series, "start_address"),
      length_(series, "length") {
  sys_call_name_ = "munmap";
}

void MunmapSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("start_address(", startAddress, "), ", "length(",
                            sizeOfMap, ")");
}

void MunmapSystemCallTraceReplayModule::processRow() {
  //find the target to unmap
  pid_t pid = executing_pid();
  VM_manager* vm_manager = VM_manager::getInstance();
  VM_area* area = vm_manager->get_VM_area(pid);

  std::vector<VM_node *> *nodes = area->find_VM_node((void *)startAddress);
  for(VM_node * node: *nodes){
    int64_t offset = startAddress - reinterpret_cast<int64_t>(node->traced_start_address);
    int64_t munmap_start = reinterpret_cast<int64_t>(node->replayed_start_address) + offset;
    int ret_val = munmap((void *)munmap_start, sizeOfMap);
    if(ret_val != 0)  //unsuccessful unmapping
      return;
  }

  bool update_vm = area->delete_VM_node((void *)startAddress, sizeOfMap);

  return;
}

void MunmapSystemCallTraceReplayModule::prepareRow() {
  startAddress = start_address_.val();
  sizeOfMap = length_.val();
  SystemCallTraceReplayModule::prepareRow();
}
