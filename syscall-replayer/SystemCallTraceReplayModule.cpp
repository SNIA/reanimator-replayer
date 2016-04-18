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
 * This file implements all the functions in the SystemCallTraceReplayModule header
 * file
 *
 * Read SystemCallTraceReplayModule.hpp for more information about this class.
 */

#include "SystemCallTraceReplayModule.hpp"

SystemCallTraceReplayModule::SystemCallTraceReplayModule(DataSeriesModule &source,
							 bool verbose_flag, 
							 int warn_level_flag):
  RowAnalysisModule(source),
  verbose_(verbose_flag),
  warn_level_(warn_level_flag),
  time_called_(series, "time_called"),
  errno_number_(series, "errno_number", Field::flag_nullable),
  return_value_(series, "return_value", Field::flag_nullable),
  completed_(false),
  replayed_ret_val_(0) {
}

bool SystemCallTraceReplayModule::verbose_mode() const {
  return verbose_;
}

bool SystemCallTraceReplayModule::default_mode() const {
  return warn_level_ == DEFAULT_MODE;
}

bool SystemCallTraceReplayModule::warn_mode() const {
  return warn_level_ == WARN_MODE;
}

bool SystemCallTraceReplayModule::abort_mode() const {
  return warn_level_ == ABORT_MODE;
}

double SystemCallTraceReplayModule::time_called() const {
  return (double)time_called_.val();
}

int SystemCallTraceReplayModule::errno_number() const {
  return (int)errno_number_.val();
}

int SystemCallTraceReplayModule::return_value() const {
  return (int)return_value_.val();
}

Extent::Ptr SystemCallTraceReplayModule::getSharedExtent() {
  Extent::Ptr e = source.getSharedExtent();
  if (e == NULL) {
    if (!completed_ && prepared) {
      completed_ = true;
      completeProcessing();
    }
    return e;
  }
  completed_ = false;
  if (!prepared) {
    firstExtent(*e);
  }
  newExtentHook(*e);
  series.setExtent(e);
  if (!prepared) {
    prepareForProcessing();
    prepared = true;
  }
  return e;
}

bool SystemCallTraceReplayModule::cur_extent_has_more_record() {
  if (series.morerecords()) {
    return true;
  } else {
    series.clearExtent();
    return false;
  }
}
  
void SystemCallTraceReplayModule::execute() {
  ++processed_rows;
  processRow();
  after_sys_call();
  ++series;
}

void SystemCallTraceReplayModule::after_sys_call() {
  compare_retval_and_errno();
  if (verbose_mode()) {
    std::cout << "System call '" << sys_call_name_ << "' was executed with following arguments:" << std::endl;
    print_sys_call_fields();
  }
}

void SystemCallTraceReplayModule::print_sys_call_fields() {
  print_common_fields();
  std::cout << ", ";
  print_specific_fields();
  std::cout << std::endl;
}

void SystemCallTraceReplayModule::print_common_fields() {
  std::cout << sys_call_name_ << ": ";
  std::cout.precision(25);
  std::cout << "time called(" << std::fixed << time_called() << "), ";
  std::cout << "errno(" << errno_number() << "), ";
  std::cout << "return value(" << return_value() << "), ";
  std::cout << "replayed return value(" << replayed_ret_val_ << ")";
}

void SystemCallTraceReplayModule::compare_retval_and_errno() {
  if (default_mode()) {
    return;
  }

  if (return_value() != replayed_ret_val_) {
    print_sys_call_fields();
    std::cout << "Warning: Return values are different.\n";
    if (abort_mode()) {
      abort();
    }
  } else if (replayed_ret_val_ == -1) {
    if (errno != errno_number()) {
      print_sys_call_fields();
      std::cout << "Warning: Errno numbers are different.\n";
      if (abort_mode()) {
	abort();
      }
    }
  }
}
