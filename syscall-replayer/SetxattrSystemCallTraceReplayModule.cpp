/*
 * Copyright (c) 2017 Darshan Godhia
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * SetxattrSystemCallTraceReplayModule header file
 *
 * Read SetxattrSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include <utility>

#include "SetxattrSystemCallTraceReplayModule.hpp"

SetxattrSystemCallTraceReplayModule::SetxattrSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag, std::string pattern_data)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      verify_(verify_flag),
      pattern_data_(std::move(pattern_data)),
      given_pathname_(series, "given_pathname", Field::flag_nullable),
      xattr_name_(series, "xattr_name", Field::flag_nullable),
      value_written_(series, "value_written", Field::flag_nullable),
      value_size_(series, "value_size"),
      flag_value_(series, "flag_value", Field::flag_nullable) {
  sys_call_name_ = "setxattr";
}

void SetxattrSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info(
      "pathname(", given_pathname_.val(), "), xattr name(", xattr_name_.val(),
      "), value written(", value_written_.val(), "), value size(",
      value_size_.val(), "), flags(", flag_value_.val(), ")");
}

void SetxattrSystemCallTraceReplayModule::processRow() {
  const char *pathname = reinterpret_cast<const char *>(given_pathname_.val());
  const char *xattr_name = reinterpret_cast<const char *>(xattr_name_.val());
  char *value = nullptr;
  size_t size = value_size_.val();
  int flags = flag_value_.val();

  if (value_written_.isNull() && pattern_data_.empty()) {
    // Write zeros by default
    pattern_data_ = "0";
  }

  // Check to see if user wants to use pattern
  if (!pattern_data_.empty()) {
    value = new char[size];
    if (pattern_data_ == "random") {
      // Fill value buffer using rand()
      value = random_fill_buffer(value, size);
    } else if (pattern_data_ == "urandom") {
      // Fill value buffer using data generated from /dev/urandom
      SystemCallTraceReplayModule::random_file_.read(value, size);
    } else {
      // Write zeros or pattern specified in pattern_data
      unsigned char pattern = pattern_data_[0];

      /*
       * XXX FUTURE WORK: Currently we support pattern of one byte
       * For multi byte pattern data, we have to modify the
       * implementation of filling value.
       */
      memset(value, pattern, size);
    }
    // replay the setxattr system call
    replayed_ret_val_ = setxattr(pathname, xattr_name, value, size, flags);
  } else {
    // Use the traced data
    auto value = reinterpret_cast<const char *>(value_written_.val());
    // replay the setxattr system call
    replayed_ret_val_ = setxattr(pathname, xattr_name, value, size, flags);
  }

  // Free the buffer
  if (!pattern_data_.empty() && value != nullptr) {
    delete[] value;
  }
}

LSetxattrSystemCallTraceReplayModule::LSetxattrSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag, std::string pattern_data)
    : SetxattrSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
                                          warn_level_flag, pattern_data) {
  sys_call_name_ = "lsetxattr";
}

void LSetxattrSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info(
      "pathname(", given_pathname_.val(), "), xattr name(", xattr_name_.val(),
      "), value written(", value_written_.val(), "), value size(",
      value_size_.val(), "), flags(", flag_value_.val(), ")");
}

void LSetxattrSystemCallTraceReplayModule::processRow() {
  const char *pathname = reinterpret_cast<const char *>(given_pathname_.val());
  const char *xattr_name = reinterpret_cast<const char *>(xattr_name_.val());
  char *value = nullptr;
  size_t size = value_size_.val();
  int flags = flag_value_.val();

  if (value_written_.isNull() && pattern_data_.empty()) {
    // Write zeros by default
    pattern_data_ = "0";
  }

  // Check to see if user wants to use pattern
  if (!pattern_data_.empty()) {
    value = new char[size];
    if (pattern_data_ == "random") {
      // Fill value buffer using rand()
      value = random_fill_buffer(value, size);
    } else if (pattern_data_ == "urandom") {
      // Fill value buffer using data generated from /dev/urandom
      SystemCallTraceReplayModule::random_file_.read(value, size);
    } else {
      // Write zeros or pattern specified in pattern_data
      unsigned char pattern = pattern_data_[0];

      /*
       * XXX FUTURE WORK: Currently we support pattern of one byte
       * For multi byte pattern data, we have to modify the
       * implementation of filling value.
       */
      memset(value, pattern, size);
    }
    // replay the setxattr system call
    replayed_ret_val_ = lsetxattr(pathname, xattr_name, value, size, flags);
  } else {
    // Use the traced data
    auto value = reinterpret_cast<const char *>(value_written_.val());
    // replay the setxattr system call
    replayed_ret_val_ = lsetxattr(pathname, xattr_name, value, size, flags);
  }

  // Free the buffer
  if (!pattern_data_.empty() && value != nullptr) {
    delete[] value;
  }
}

FSetxattrSystemCallTraceReplayModule::FSetxattrSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag, std::string pattern_data)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      verify_(verify_flag),
      pattern_data_(std::move(pattern_data)),
      descriptor_(series, "descriptor"),
      xattr_name_(series, "xattr_name", Field::flag_nullable),
      value_written_(series, "value_written", Field::flag_nullable),
      value_size_(series, "value_size"),
      flag_value_(series, "flag_value", Field::flag_nullable) {
  sys_call_name_ = "fsetxattr";
}

void FSetxattrSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info("traced fd(", descriptor_.val(), "), replayed fd(",
                            replayed_fd, "), xattr name(", xattr_name_.val(),
                            "), value written(", value_written_.val(),
                            "), value size(", value_size_.val(), "), flags(",
                            flag_value_.val(), ")");
}

void FSetxattrSystemCallTraceReplayModule::processRow() {
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  const char *xattr_name = reinterpret_cast<const char *>(xattr_name_.val());
  char *value = nullptr;
  size_t size = value_size_.val();
  int flags = flag_value_.val();

  if (fd == SYSCALL_SIMULATED) {
    /*
     * FD for the fsetxattr system call originated from a socket().
     * The system call will not be replayed.
     * Original return value will be returned.
     */
    replayed_ret_val_ = return_value_.val();
    return;
  }
  // Check to see if user wants to use pattern
  if (!pattern_data_.empty()) {
    value = new char[size];
    if (pattern_data_ == "random") {
      // Fill value buffer using rand()
      value = random_fill_buffer(value, size);
    } else if (pattern_data_ == "urandom") {
      // Fill value buffer using data generated from /dev/urandom
      SystemCallTraceReplayModule::random_file_.read(value, size);
    } else {
      // Write zeros or pattern specified in pattern_data
      unsigned char pattern = pattern_data_[0];

      /*
       * XXX FUTURE WORK: Currently we support pattern of one byte
       * For multi byte pattern data, we have to modify the
       * implementation of filling value.
       */
      memset(value, pattern, size);
    }
    // replay the fsetxattr system call
    replayed_ret_val_ = fsetxattr(fd, xattr_name, value, size, flags);
  } else {
    // Use the traced data
    auto value = reinterpret_cast<const char *>(value_written_.val());
    // replay the fsetxattr system call
    replayed_ret_val_ = fsetxattr(fd, xattr_name, value, size, flags);
  }

  // Free the buffer
  if (!pattern_data_.empty() && value != nullptr) {
    delete[] value;
  }
}
