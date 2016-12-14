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
 * This file implements all the functions in the
 * ReadSystemCallTraceReplayModule header file
 *
 * Read ReadSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "ReadSystemCallTraceReplayModule.hpp"

ReadSystemCallTraceReplayModule::
ReadSystemCallTraceReplayModule(DataSeriesModule &source,
				bool verbose_flag,
				bool verify_flag,
				int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  verify_(verify_flag),
  descriptor_(series, "descriptor"),
  data_read_(series, "data_read", Field::flag_nullable),
  bytes_requested_(series, "bytes_requested") {
  sys_call_name_ = "read";
}

void ReadSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("descriptor:", descriptor_.val(), "), ", \
	   "data read(", data_read_.val(), "), ", \
	   "bytes requested(", bytes_requested_.val(), ")");
}

void ReadSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  int nbytes = bytes_requested_.val();
  char buffer[nbytes];
  replayed_ret_val_ = read(fd, buffer, nbytes);

  if (verify_ == true) {
    // Verify read data and data in the trace file are same
    if (memcmp(data_read_.val(), buffer, replayed_ret_val_) != 0) {
      // Data aren't same
      syscall_logger_->log_err("Verification of data in read failed.");
      if (!default_mode()) {
        syscall_logger_->log_warn("time called:", \
	  boost::format(DEC_PRECISION) % Tfrac_to_sec(time_called()), \
          " Captured read data is different from replayed read data");
        syscall_logger_->log_warn("Captured read data: ", data_read_.val(), ", ", \
          "Replayed read data: ", std::string(buffer));
        if (abort_mode()) {
          abort();
        }
      }
    } else {
      if (verbose_mode()) {
        syscall_logger_->log_info("Verification of data in read success.");
      }
    }
  }
}

PReadSystemCallTraceReplayModule::
PReadSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 bool verify_flag,
				 int warn_level_flag):
  ReadSystemCallTraceReplayModule(source, verbose_flag,
				  verify_flag, warn_level_flag),
  offset_(series, "offset") {
  sys_call_name_ = "pread";
}

void PReadSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("descriptor:", descriptor_.val(), "), ", \
    "data read(", data_read_.val(), "), ", \
    "bytes requested(", bytes_requested_.val(), "), " \
    "offset(", offset_.val(), ")");
}

void PReadSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  int nbytes = bytes_requested_.val();
  char buffer[nbytes];
  int offset = offset_.val();
  replayed_ret_val_ = pread(fd, buffer, nbytes, offset);

  if (verify_ == true) {
    // Verify read data and data in the trace file are same
    if (memcmp(data_read_.val(), buffer, replayed_ret_val_) != 0) {
      // Data aren't same
      syscall_logger_->log_err("Verification of data in pread failed.");
      if (!default_mode()) {
        syscall_logger_->log_warn("time called:", \
	  boost::format(DEC_PRECISION) % Tfrac_to_sec(time_called()), \
          " Captured pread data is different from replayed pread data");
        syscall_logger_->log_warn("Captured pread data: ", data_read_.val(), ", ", \
          "Replayed pread data: ", std::string(buffer));
        if (warn_level_ == ABORT_MODE ) {
          abort();
        }
      }
    } else {
      if (verbose_) {
        syscall_logger_->log_info("Verification of data in pread success.");
      }
    }
  }
}
