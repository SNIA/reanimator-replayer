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
 * ReadvSystemCallTraceReplayModule header file
 *
 * Read ReadvSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "ReadvSystemCallTraceReplayModule.hpp"

ReadvSystemCallTraceReplayModule::
ReadvSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 bool verify_flag,
				 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  verify_(verify_flag),
  descriptor_(series, "descriptor", Field::flag_nullable),
  count_(series, "count", Field::flag_nullable),
  iov_number_(series, "iov_number"),
  data_read_(series, "data_read", Field::flag_nullable),
  bytes_requested_(series, "bytes_requested") {
  sys_call_name_ = "readv";
}

void ReadvSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "System call '" << sys_call_name_
	    << "' was executed with following arguments:" << std::endl;
  // Print common fields first
  print_common_fields();
  std::cout << ", " << std::endl;

  /*
   * Then, print the descriptor value, number of iovec and total
   * number of bytes read from the first record of dataseries file.
   */
  std::cout << "descriptor:(" << descriptor_.val() << "), ";
  std::cout << "count:(" << count_.val() << "), ";
  std::cout << "bytes requested:(" << bytes_requested_.val() << ")\n";

  int count = count_.val();

  /*
   * Iteratively fetch the new record to print the iov_number
   * and data read for each iovec buffers.
   */
  while(count > 0 && series.morerecords()) {
    ++series;
    std::cout << "iov_number:(" << iov_number_.val() << "), ";
    std::cout << "data_read:(" << data_read_.val() << ")\n";
    count--;
  }
  std::cout << std::endl;
}

void ReadvSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Readv System Call Replayer starts to replay...-----"
	    << std::endl;
}

void ReadvSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  int count = count_.val(); /* Number of read io vectors */
  int iov_number = iov_number_.val();
  char *traced_buffer[count];
  char *replayed_buffer[count];
  int actual_ret_val_ = return_value();

  struct iovec iov[count];

  /*
   * Save the position of the first record in the Extent Series.
   */
  const void *first_record_pos = series.getCurPos();
  int iovcnt = count;

  /*
   * If iov number is equal to '-1', this means it is first record of
   * single readv system call.
   */
  if (iov_number == -1) {
    /*
     * Iteratively fetch the records and save the bytes_requested, iov
     * number and actual buffer for each record of single readv system
     * call.
     */
    while (iovcnt > 0 && series.morerecords()) {
      ++processed_rows;
      ++series; /* This moves the pointer in extent series to next record */

      int iov_num = iov_number_.val();
      size_t bytes_requested = bytes_requested_.val();

      /*
       * Allocate memory and copy the actual buffer.
       * XXX NOTE: *****FUTURE WORK *****
       * Instead of allocating individual buffer, we can allocate
       * one single buffer.
       */
      traced_buffer[iov_num] = new char[bytes_requested];
      memcpy(traced_buffer[iov_num], data_read_.val(), bytes_requested);

      /*
       * Allocate memory for replaying buffers and construct the
       * struct iovec from each record.
       */
      replayed_buffer[iov_num] = new char[bytes_requested];
      iov[iov_num].iov_base = replayed_buffer[iov_num];
      iov[iov_num].iov_len = bytes_requested;

      iovcnt--;
    }
  }

  /*
   * Replay the readv system call.
   */
  replayed_ret_val_ = readv(fd, iov, count);

  /* If replayer runs in verify mode. */
  if (verify_ == true) {
    // Verify each iovec read data and data in the trace file
    for (int iovcnt_ = 0; iovcnt_ < count; iovcnt_++) {
      if (memcmp(traced_buffer[iovcnt_],
		 replayed_buffer[iovcnt_],
		 iov[iovcnt_].iov_len) != 0) {
	//Data aren't same
	std::cerr << "Verification of data for iov number: "
		  << iovcnt_ << " in readv failed.\n";
	if (!default_mode()) {
	  std::cout << "time called:" << std::fixed
		    << Tfrac_to_sec(time_called()) << std::endl;
	  std::cout << "Captured readv data is different from replayed"
		    << "read data" << std::endl;
	  std::cout << "Captured readv data: " << traced_buffer[iovcnt_] << ", ";
	  std::cout << "Replayed readv data: " << replayed_buffer[iovcnt_]
		    << std::endl;
	  if (abort_mode()) {
	    abort();
	  }
	}
      } else if (verbose_mode()) {
	std::cout << "Verification of data for iov number: "
		  << iovcnt_ << " in readv succeeded" << std::endl;
      }
    }
  }

  // Save the position of the last record in Extent Series.
  const void *last_record_pos = series.getCurPos();

  /* If replayer runs in verbose mode. */
  if (verbose_mode()) {
    // Sets the position to the first record of single readv system call.
    series.setCurPos(first_record_pos);
    print_specific_fields();
    // Sets the position to the last record of single readv system call.
    series.setCurPos(last_record_pos);
  }

  /*
   * If return values are different, then check for
   * default, warn and abort mode.
   */
  if (actual_ret_val_ != replayed_ret_val_) {
    if (default_mode())
      return;
    series.setCurPos(first_record_pos);
    print_specific_fields();
    series.setCurPos(last_record_pos);
    std::cout << "Warning: Return values are different. \n";
    if (abort_mode())
      abort();
  }

  /*
   * Free both traced and replayed buffers.
   */
  for (int iovcnt_ = 0; iovcnt_ < count; iovcnt_++) {
    delete[] replayed_buffer[iovcnt_];
    delete[] traced_buffer[iovcnt_];
  }
}

void ReadvSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Readv System Call Replayer finished replaying...-----"
	    << std::endl;
}
