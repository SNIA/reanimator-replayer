/*
 * Copyright (c) 2016 Nina Brown
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

std::string SystemCallTraceReplayModule::sys_call_name() const {
  return sys_call_name_;
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

int64_t SystemCallTraceReplayModule::return_value() const {
  return (int64_t)return_value_.val();
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
      syscall_logger_->log_info("---'", sys_call_name_, \
			"' System Call Replayer has started replaying---");
      prepared = true;
    }
  } else if (prepared) {
      syscall_logger_->log_info("---'", sys_call_name_, \
			"' System Call Replayer has finished replaying---");
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
  /*
   * If a system call is being replayed by the syscall-replayer
   * program, then compare the errno numbers and return values,
   * else skip comparing them.
   */
  if (isReplayable())
    compare_retval_and_errno();
  if (verbose_mode()) {
    syscall_logger_->log_info("System call '", sys_call_name_, \
		     "' was executed with following arguments:", \
		     sys_call_name_ , ": ");
    print_sys_call_fields();
  }
}

void SystemCallTraceReplayModule::print_sys_call_fields() {
  print_common_fields();
  print_specific_fields();
}

void SystemCallTraceReplayModule::print_common_fields() {
  // Convert the time values from Tfracs to seconds
  double time_called_val = Tfrac_to_sec(time_called());
  double time_returned_val = Tfrac_to_sec(time_returned());
  double time_recorded_val = Tfrac_to_sec(time_recorded());

  // Print the common fields and their values
  syscall_logger_->log_info("time called(", formatVal(time_called_val, std::fixed), "), ", \
		   "time returned(", formatVal(time_returned_val, std::fixed), "), ", \
		   "time recorded(", formatVal(time_recorded_val, std::fixed), "), ", \
		   "executing pid(", executing_pid(), "), ", \
		   "errno(", errno_number(), "), ", \
		   "return value(", return_value(), "), ", \
		   "replayed return value(", replayed_ret_val_, "), ", \
		   "unique id(", unique_id_.val(), ")");
}

void SystemCallTraceReplayModule::compare_retval_and_errno() {
  if (default_mode()) {
    return;
  }

  if (return_value() != replayed_ret_val_) {
    syscall_logger_->log_warn(sys_call_name_, " syscall has different return values");
    print_sys_call_fields();
    syscall_logger_->log_warn("Return values are different.");
    if (abort_mode()) {
      abort();
    }
  } else if (replayed_ret_val_ == -1) {
    if (errno != errno_number()) {
      syscall_logger_->log_warn(sys_call_name_, " syscall has different errno number");
      print_sys_call_fields();
      syscall_logger_->log_warn("Errno numbers are different.");
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
  uint32_t u_secs = (uint32_t) ((time_in_secs - (double) full_secs)
				* pow(10.0, 6));
  tv.tv_sec = full_secs;
  tv.tv_usec = u_secs;
  return tv;
}

struct timespec SystemCallTraceReplayModule::Tfrac_to_timespec(uint64_t time) {
  struct timespec ts;
  double time_in_secs = Tfrac_to_sec(time);
  uint32_t full_secs = (uint32_t) time_in_secs;
  uint32_t n_secs = (uint32_t) ((time_in_secs - (double) full_secs)
				* pow(10.0, 9));
  ts.tv_sec = full_secs;
  ts.tv_nsec = n_secs;
  return ts;
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

/*
 * Some system calls such as _exit, execve, mmap and munmap are not
 * appropriate to replay. So we do not replay in our replayer.
 *
 * @return: returns true if the system call is replayable, else it
 *	      returns false.
 */
bool SystemCallTraceReplayModule::isReplayable() {
  if (sys_call_name_ == "exit" ||
      sys_call_name_ == "execve" ||
      sys_call_name_ == "mmap" ||
      sys_call_name_ == "munmap")
    return false;
  return true;
}

/*
 * This function takes a value and converts it to string representation
 * of given base. This function is only used while printing the values
 * of system call arguments.
 */
std::string SystemCallTraceReplayModule::formatVal(double value,
					std::ios_base &(base)(std::ios_base&)) {
  std::stringstream oss;
  if (base == std::hex)
    oss << "0x";
  else if (base == std::oct)
    oss << "0";
  else if (base == std::fixed)
    oss.precision(25);
  oss << base << value;
  return oss.str();
}
