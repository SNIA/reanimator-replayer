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
  return_value_(series, "return_value", Field::flag_nullable),
  completed_(false) {
}
  
double SystemCallTraceReplayModule::time_called() const {
  return (double)time_called_.val();
}

Extent::Ptr SystemCallTraceReplayModule::getSharedExtent() {
  Extent::Ptr e = source.getSharedExtent();
  if (e == NULL) {
    if (!completed_) {
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
  ++series;
}

void SystemCallTraceReplayModule::compare_retval(int ret_val) {
  if (verbose_){
    std::cout << "Captured return value: " << return_value_.val() << ", ";
    std::cout << "Replayed return value: " << ret_val << std::endl;
  }
  
  if (warn_level_ != DEFAULT_MODE && return_value_.val() != ret_val) {
    std::cout << "time called:" << std::fixed << time_called() << std::endl;
    std::cout << "Captured return value is different from replayed return value" << std::endl;
    std::cout << "Captured return value: " << return_value_.val() << ", ";
    std::cout << "Replayed return value: " << ret_val << std::endl;
    if (warn_level_ == ABORT_MODE) {
      abort();
    }
  }
}
