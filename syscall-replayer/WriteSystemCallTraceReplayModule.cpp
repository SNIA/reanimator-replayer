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
 * WriteSystemCallTraceReplayModule header file
 *
 * Read WriteSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "WriteSystemCallTraceReplayModule.hpp"

WriteSystemCallTraceReplayModule::
WriteSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 bool verify_flag,
				 int warn_level_flag,
				 std::string pattern_data):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  verify_(verify_flag),
  pattern_data_(pattern_data),
  descriptor_(series, "descriptor"),
  data_written_(series, "data_written", Field::flag_nullable),
  bytes_requested_(series, "bytes_requested") {
  sys_call_name_ = "write";
}

void WriteSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("descriptor(", descriptor_.val(), "), " \
	   "data(", data_written_.val(), "), ", \
	   "nbytes(", bytes_requested_.val(), ")");
}

void WriteSystemCallTraceReplayModule::processRow() {
  size_t nbytes = bytes_requested_.val();
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  char *data_buffer;

  // Check to see if user wants to use pattern
  if (!pattern_data_.empty()) {
    data_buffer = new char[nbytes];
    if (pattern_data_ == "random") {
      // Fill write buffer using rand()
      data_buffer = random_fill_buffer(data_buffer, nbytes);
    } else if (pattern_data_ == "urandom") {
      // Fill write buffer using data generated from /dev/urandom
      SystemCallTraceReplayModule::random_file_.read(data_buffer, nbytes);
    } else {
      // Write zeros or pattern specified in pattern_data
      unsigned char pattern = pattern_data_[0];

      /*
       * XXX FUTURE WORK: Currently we support pattern of one byte.
       * For multi byte pattern data, we have to modify the
       * implementation of filling data_buffer.
       */
      memset(data_buffer, pattern, nbytes);
    }
  } else {
    // Write the traced data
    data_buffer = (char *)data_written_.val();
  }

  replayed_ret_val_ = write(fd, data_buffer, nbytes);

  // Free the buffer
  if (!pattern_data_.empty()){
    delete[] data_buffer;
  }
}

PWriteSystemCallTraceReplayModule::
PWriteSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  bool verify_flag,
				  int warn_level_flag,
				  std::string pattern_data):
  WriteSystemCallTraceReplayModule(source, verbose_flag,
				   verify_flag, warn_level_flag, pattern_data),
  offset_(series, "offset") {
  sys_call_name_ = "pwrite";
}

void PWriteSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("descriptor(", descriptor_.val(), "), ", \
	   "data(", data_written_.val(), "), ", \
	   "nbytes(", bytes_requested_.val(), "), ", \
	   "offset(", offset_.val(), ")");
}

void PWriteSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  size_t nbytes = bytes_requested_.val();
  char *data_buffer;

  // Check to see if write data is NULL in DS or user didn't specify pattern
  if (data_written_.isNull() && pattern_data_.empty() ) {
    // Let's write zeros.
    pattern_data_ = "0x0";
  }

  // Check to see if user wants to use pattern
  if (!pattern_data_.empty()) {
    data_buffer = new char[nbytes];
    if (pattern_data_ == "random") {
      // Fill write buffer using rand()
      data_buffer = random_fill_buffer(data_buffer, nbytes);
    } else if (pattern_data_ == "urandom") {
      // Fill write buffer using data generated from /dev/urandom
      SystemCallTraceReplayModule::random_file_.read(data_buffer, nbytes);
    } else {
      // Write zeros or pattern specified in pattern_data
      unsigned char pattern = pattern_data_[0];

      /*
       * XXX FUTURE WORK: Currently we support pattern of one byte.
       * For multi byte pattern data, we have to modify the
       * implementation of filling data_buffer.
       */
      memset(data_buffer, pattern, nbytes);
    }
  } else {
    // Write the traced data
    data_buffer = (char *)data_written_.val();
  }

  int offset = offset_.val();
  replayed_ret_val_ = pwrite(fd, data_buffer, nbytes, offset);

  // Free the buffer
  if (!pattern_data_.empty())
    delete[] data_buffer;
}
