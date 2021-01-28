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
 * ReadlinkSystemCallTraceReplayModule header file
 *
 * Read ReadlinkSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "ReadlinkSystemCallTraceReplayModule.hpp"

ReadlinkSystemCallTraceReplayModule::ReadlinkSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      verify_(verify_flag),
      given_pathname_(series, "given_pathname", Field::flag_nullable),
      link_value_(series, "link_value", Field::flag_nullable),
      buffer_size_(series, "buffer_size", Field::flag_nullable) {
  sys_call_name_ = "readlink";
}

void ReadlinkSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("link path(", pathname, "), ", "target path(",
                            dataReadBuf, "), ", "buffer size(", nbytes, ")");
}

void ReadlinkSystemCallTraceReplayModule::processRow() {
  // replay the readlink system call
  replayed_ret_val_ = readlink(pathname, buffer, nbytes);

  if (verify_) {
    // Verify readlink buffer and buffer in the trace file are same
    if (dataReadBuf != nullptr && memcmp(dataReadBuf, buffer, returnVal) != 0) {
      // Target path aren't same
      syscall_logger_->log_err("Verification of path in readlink failed.");
      if (!default_mode()) {
        syscall_logger_->log_warn(
            "time called:",
            boost::format(DEC_PRECISION) % Tfrac_to_sec(time_called()),
            ", Captured readlink path is different from",
            " replayed readlink path");
        syscall_logger_->log_warn("Captured readlink path: ", dataReadBuf, ", ",
                                  "Replayed readlink path: ", buffer);
        if (abort_mode()) {
          abort();
        }
      }
    } else {
      if (verbose_mode()) {
        syscall_logger_->log_info("Verification of path in readlink success.");
      }
    }

    delete[] dataReadBuf;
  }

  delete[] buffer;
}

void ReadlinkSystemCallTraceReplayModule::prepareRow() {
  auto pathBuf = reinterpret_cast<const char *>(given_pathname_.val());
  pathname = copyPath(pathBuf);
  nbytes = buffer_size_.val();
  replayed_ret_val_ = return_value_.val();
  buffer = new char[nbytes];
  if (verify_) {
    auto dataBuf = reinterpret_cast<const char *>(link_value_.val());
    if (replayed_ret_val_ > 0) {
      dataReadBuf = new char[replayed_ret_val_];
      std::memcpy(dataReadBuf, dataBuf, replayed_ret_val_);
    } else {
      dataReadBuf = nullptr;
    }
  }
  SystemCallTraceReplayModule::prepareRow();
}
