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
 * This file implements all the functions in the SystemCallTraceReplayModule
 * header file
 *
 * Read SystemCallTraceReplayModule.hpp for more information about this class.
 */

#include "SystemCallTraceReplayModule.hpp"

SystemCallTraceReplayModule::SystemCallTraceReplayModule(DataSeriesModule
							 &source,
							 bool verbose_flag,
							 int warn_level_flag):
  RowAnalysisModule(source),
  verbose_(verbose_flag),
  warn_level_(warn_level_flag),
  time_called_(series, "time_called", Field::flag_nullable),
  time_returned_(series, "time_returned", Field::flag_nullable),
  time_recorded_(series, "time_recorded", Field::flag_nullable),
  executing_pid_(series, "executing_pid", Field::flag_nullable),
  errno_number_(series, "errno_number", Field::flag_nullable),
  return_value_(series, "return_value", Field::flag_nullable),
  unique_id_(series, "unique_id"),
  rows_per_call_(1),
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

uint64_t SystemCallTraceReplayModule::time_called() const {
  return (uint64_t)time_called_.val();
}

uint64_t SystemCallTraceReplayModule::time_returned() const {
  return (uint64_t)time_returned_.val();
}

uint64_t SystemCallTraceReplayModule::time_recorded() const {
  return (uint64_t)time_recorded_.val();
}

uint32_t SystemCallTraceReplayModule::executing_pid() const {
  return (uint32_t)executing_pid_.val();
}

int SystemCallTraceReplayModule::errno_number() const {
  return (int)errno_number_.val();
}

int SystemCallTraceReplayModule::return_value() const {
  return (int)return_value_.val();
}

int64_t SystemCallTraceReplayModule::unique_id() const {
  return (int64_t)unique_id_.val();
}

Extent::Ptr SystemCallTraceReplayModule::getSharedExtent() {
  Extent::Ptr e = source.getSharedExtent();
  if (e != NULL) {
    if (!prepared) {
      firstExtent(*e);
    }
    newExtentHook(*e);
    series.setExtent(e);
    if (!prepared) {
      std::cout << "----- '" << sys_call_name_ << "' "
                << "System Call Replayer has started replaying...-----\n"
                << std::endl;
      prepared = true;
    }
  } else if (prepared) {
      std::cout << "----- '" << sys_call_name_ << "' "
		<< "System Call Replayer finished replaying...-----\n"
		<< std::endl;
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
  processRow();
  completeProcessing();
}

void SystemCallTraceReplayModule::completeProcessing() {
  after_sys_call();
  for (int i = 0; i < rows_per_call_; i++)
    ++series;
}

void SystemCallTraceReplayModule::after_sys_call() {
  compare_retval_and_errno();
  if (verbose_mode()) {
    std::cout << "System call '" << sys_call_name_ <<
      "' was executed with following arguments:" << std::endl;
    std::cout << sys_call_name_ << ": " << std::endl;
    print_sys_call_fields();
    std::cout << std::endl;
  }
}

void SystemCallTraceReplayModule::print_sys_call_fields() {
  print_common_fields();
  std::cout << ", " << std::endl;
  print_specific_fields();
  std::cout << std::endl;
}

void SystemCallTraceReplayModule::print_common_fields() {
  // Convert the time values from Tfracs to seconds
  double time_called_val = Tfrac_to_sec(time_called());
  double time_returned_val = Tfrac_to_sec(time_returned());
  double time_recorded_val = Tfrac_to_sec(time_recorded());

  // Print the common fields and their values
  std::cout.precision(25);
  std::cout << "time called(" << std::fixed << time_called_val << "), "
	    << std::endl;
  std::cout << "time returned(" << std::fixed << time_returned_val << "), "
	    << std::endl;
  std::cout << "time recorded(" << std::fixed << time_recorded_val << "), "
	    << std::endl;
  std::cout << "executing pid(" << executing_pid() << "), " << std::endl;
  std::cout << "errno(" << errno_number() << "), " << std::endl;
  std::cout << "return value(" << return_value() << "), " << std::endl;
  std::cout << "replayed return value(" << replayed_ret_val_ << "), "
	    << std::endl;
  std::cout << "unique id(" << unique_id_.val() << ")";
}

void SystemCallTraceReplayModule::compare_retval_and_errno() {
  if (default_mode()) {
    return;
  }

  if (return_value() != replayed_ret_val_) {
    std::cout << sys_call_name_ << ": " << std::endl;
    print_sys_call_fields();
    std::cout << "Warning: Return values are different.\n";
    if (abort_mode()) {
      abort();
    }
  } else if (replayed_ret_val_ == -1) {
    if (errno != errno_number()) {
      std::cout << sys_call_name_ << ": " << std::endl;
      print_sys_call_fields();
      std::cout << "Warning: Errno numbers are different.\n";
      if (abort_mode()) {
	abort();
      }
    }
  }
}

double SystemCallTraceReplayModule::Tfrac_to_sec(uint64_t time) {
  double time_in_secs = (double)(time * pow(2.0, -32));
  return time_in_secs;
}

struct timeval SystemCallTraceReplayModule::Tfrac_to_timeval(uint64_t time) {
  struct timeval tv;
  double time_in_secs = Tfrac_to_sec(time);
  uint32_t full_secs = (uint32_t) time_in_secs;
  uint32_t u_secs = (uint32_t) (time_in_secs - (double) full_secs)
    * pow(10.0, 6);
  tv.tv_sec = full_secs;
  tv.tv_usec = u_secs;
  return tv;
}

/*
 * This function will be used to fill buffer with randomly
 * generated bytes using rand() and srand().
 */
char *SystemCallTraceReplayModule::random_fill_buffer(char *buffer,
						      size_t nbytes) {
  size_t size = sizeof(size_t);
  size_t remaining = nbytes % size;
  size_t bytes = nbytes - remaining;
  size_t i, num;
  srand(time(0));
  num = rand();
  memcpy(buffer, &num, remaining);
  for (i = remaining; i < bytes; i += size) {
    num = rand();
    memcpy(buffer + i, &num, size);
  }
  return buffer;
}
