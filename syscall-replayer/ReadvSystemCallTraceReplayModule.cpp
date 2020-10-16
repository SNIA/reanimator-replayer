/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
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

ReadvSystemCallTraceReplayModule::ReadvSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      verify_(verify_flag),
      descriptor_(series, "descriptor", Field::flag_nullable),
      count_(series, "count", Field::flag_nullable),
      iov_number_(series, "iov_number"),
      data_read_(series, "iov_data_read", Field::flag_nullable),
      bytes_requested_(series, "bytes_requested") {
  sys_call_name_ = "readv";
}

void ReadvSystemCallTraceReplayModule::print_specific_fields() {
  // Save the postion of first record.
  const void *first_record_pos = series.getCurPos();

  /*
   * Print the descriptor value, number of iovec and total
   * number of bytes read from the first record of dataseries file.
   */
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info("traced fd(", descriptor_.val(), "), ",
                            "replayed fd(", replayed_fd, "), ", "count:(",
                            count_.val(), "), ", "bytes requested:(",
                            bytes_requested_.val(), ")");

  int count = count_.val();

  /*
   * Iteratively fetch the new record to print the iov_number
   * and data read for each iovec buffers.
   */
  while (count > 0 && series.morerecords()) {
    ++series;
    syscall_logger_->log_info("iov_number:(", iov_number_.val(), "), ",
                              "data_read:(", data_read_.val(), ")");
    count--;
  }

  // Again, set the pointer to the first record.
  series.setCurPos(first_record_pos);
}

void ReadvSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  int count = count_.val(); /* Number of read io vectors */
  int iov_number = iov_number_.val();
  auto traced_buffer = new char *[count];
  auto replayed_buffer = new char *[count];

  /*
   * The total number of rows processed by single readv system
   * call is one plus number of iovec which is equal to the
   * count field as described in SNIA document for readv system
   * call.
   */
  rows_per_call_ = count + 1;

  // Save the position of the first record in the Extent Series.
  const void *first_record_pos = series.getCurPos();
  int iovcnt = count;

  struct iovec iov[count];

  if (fd == SYSCALL_SIMULATED) {
    /*
     * FD for the readv call originated from a socket().
     * The system call will not be replayed.
     * Original return value and data will be returned.
     */
    replayed_ret_val_ = return_value_.val();
    return;
  }

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
      // This moves the pointer in extent series to next record
      ++series;

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

  //  Replay the readv system call.
  replayed_ret_val_ = readv(fd, iov, count);

  // If replayer runs in verify mode.
  if (verify_) {
    // Verify each iovec read data and data in the trace file
    for (int iovcnt_ = 0; iovcnt_ < count; iovcnt_++) {
      if (memcmp(traced_buffer[iovcnt_], replayed_buffer[iovcnt_],
                 iov[iovcnt_].iov_len) != 0) {
        // Data aren't same
        syscall_logger_->log_err("Verification of data for iov number: ",
                                 iovcnt_, " in readv failed.");
        if (!default_mode()) {
          syscall_logger_->log_warn(
              "time called:",
              boost::format(DEC_PRECISION) % Tfrac_to_sec(time_called()),
              ", Captured readv data is different from", " replayed read data");
          syscall_logger_->log_warn(
              "Captured readv data: ", traced_buffer[iovcnt_],
              ", Replayed readv data: ", replayed_buffer[iovcnt_]);
          if (abort_mode()) {
            abort();
          }
        }
      } else if (verbose_mode()) {
        syscall_logger_->log_info("Verification of data for iov number: ",
                                  iovcnt_, " in readv succeeded.");
      }
    }
  }

  // Free both traced and replayed buffers.
  for (int iovcnt_ = 0; iovcnt_ < count; iovcnt_++) {
    delete[] replayed_buffer[iovcnt_];
    delete[] traced_buffer[iovcnt_];
  }

  /*
   * After executing a single readv system call, set the
   * pointer in the Extent Series to the first record.
   */
  series.setCurPos(first_record_pos);

  delete[] traced_buffer;
  delete[] replayed_buffer;
}
