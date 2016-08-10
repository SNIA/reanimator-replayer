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
 * WritevSystemCallTraceReplayModule header file
 *
 * Read WritevSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "WritevSystemCallTraceReplayModule.hpp"

WritevSystemCallTraceReplayModule::
WritevSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 int warn_level_flag,
				 std::string pattern_data):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  pattern_data_(pattern_data),
  descriptor_(series, "descriptor", Field::flag_nullable),
  count_(series, "count", Field::flag_nullable),
  iov_number_(series, "iov_number"),
  data_written_(series, "data_written", Field::flag_nullable),
  bytes_requested_(series, "bytes_requested") {
  sys_call_name_ = "writev";
}

void WritevSystemCallTraceReplayModule::print_specific_fields() {
  // Save the postion of first record.
  const void *first_record_pos = series.getCurPos();

  /*
   * Print the descriptor value, number of iovec and total
   * number of bytes written from the first record of dataseries file.
   */
  std::cout << "descriptor:(" << descriptor_.val() << "), ";
  std::cout << "count:(" << count_.val() << "), ";
  std::cout << "bytes requested:(" << bytes_requested_.val() << ")\n";

  int count = count_.val();

  /*
   * Iteratively fetch the new record to print the iov_number
   * and data written for each iovec buffers.
   */
  while (count > 0 && series.morerecords()) {
    ++series;
    std::cout << "\niov_number:(" << iov_number_.val() << "), ";
    std::cout << "data_written:(" << data_written_.val() << ")";
    count--;
  }

  // Again, set the pointer to the first record.
  series.setCurPos(first_record_pos);
}

void WritevSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  int count = count_.val(); /* Number of write io vectors */
  int iov_number = iov_number_.val();
  char *data_buffer[count];

  /*
   * The total number of rows processed by single readv system
   * call is one plus number of iovec which is equal to the
   * count field as described in SNIA document for readv system
   * call.
   */
  rows_per_call_ = count + 1;

  /*
   * Save the position of the first record in the Extent Series.
   */
  const void *first_record_pos = series.getCurPos();
  int iovcnt = count;

  struct iovec iov[count];

  /*
   * If iov number is equal to '-1', this means it is first record of
   * single writev system call.
   */
  if (iov_number == -1) {
    /*
     * Iteratively fetch the records and save the bytes_requested, iov
     * number and actual buffer for each record of single writev system
     * call.
     */
    while (iovcnt > 0 && series.morerecords()) {
      ++series; /* This moves the pointer in extent series to next record */

      int iov_num = iov_number_.val();
      size_t bytes_requested = bytes_requested_.val();

      // Check to see if user wants to use pattern
      if (!pattern_data_.empty()) {
	/*
	 * Allocate memory and copy the actual buffer.
	 * XXX NOTE: ***** FUTURE WORK *****
	 * Instead of allocating individual buffer, we can allocate
	 * one single buffer.
	 */
	data_buffer[iov_num] = new char[bytes_requested];
	if (pattern_data_ == "random") {
	  // Fill write buffer using rand()
	  data_buffer[iov_num] = random_fill_buffer(data_buffer[iov_num],
						    bytes_requested);
	} else if (pattern_data_ == "urandom") {
	  // Fill write buffer using data generated from /dev/urandom
	  SystemCallTraceReplayModule::random_file_.read(data_buffer[iov_num],
							 bytes_requested);
	} else {
	  // Write zeros or pattern specified in pattern_data
	  unsigned char pattern = pattern_data_[0];

	  /*
	   * XXX FUTURE WORK: Currently we support pattern of one byte.
	   * For multi byte pattern data, we have to modify the
	   * implementation of filling data_buffer.
	   */
	  memset(data_buffer[iov_num], pattern, bytes_requested);
	}
      } else {
	// Write the traced data
	data_buffer[iov_num] = (char *)data_written_.val();
      }

      /*
       * Construct the struct iovec from each record.
       */
      iov[iov_num].iov_base = data_buffer[iov_num];
      iov[iov_num].iov_len = bytes_requested;

      iovcnt--;
    }
  }

  /*
   * Replay the writev system call.
   */
  replayed_ret_val_ = writev(fd, iov, count);

  /*
   * Free data buffer.
   */
  for (int iovcnt_ = 0; iovcnt_ < count; iovcnt_++) {
    if (!pattern_data_.empty())
      delete[] data_buffer[iovcnt_];
  }

  /*
   * After executing a single writev system call, set the
   * pointer in the Extent Series to the first record.
   */
  series.setCurPos(first_record_pos);
}
