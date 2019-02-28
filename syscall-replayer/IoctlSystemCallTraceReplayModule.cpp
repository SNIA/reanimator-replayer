/*
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * IoctlSystemCallTraceReplayModule header file
 *
 * Read IoctlSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "IoctlSystemCallTraceReplayModule.hpp"

IoctlSystemCallTraceReplayModule::IoctlSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      descriptor_(series, "descriptor"),
      request_(series, "request"),
      parameter_(series, "parameter", Field::flag_nullable),
      ioctl_buffer_(series, "ioctl_buffer", Field::flag_nullable),
      buffer_size_(series, "buffer_size") {
  sys_call_name_ = "ioctl";
}

void IoctlSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, file_descriptor);
  syscall_logger_->log_info("traced fd(", file_descriptor, "), ",
                            "replayed fd(", replayed_fd, "), ", "request(",
                            boost::format("0x%02x") % req, ")");
}

void IoctlSystemCallTraceReplayModule::processRow() {
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, file_descriptor);
  u_long request = req;
  int parameter;

  if (fd == SYSCALL_SIMULATED) {
    /*
     * FD for the ioctl originated from a socket.
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value();
    return;
  }

  // If there is no buffer data, pass the parameter value as the third argument
  if (buffer == nullptr) {
    parameter = params;
    replayed_ret_val_ = ioctl(fd, request, parameter);
  } else {
    replayed_ret_val_ = ioctl(fd, request, buffer);
    delete[] buffer;
  }
}

void IoctlSystemCallTraceReplayModule::prepareRow() {
  file_descriptor = descriptor_.val();
  req = request_.val();
  params = parameter_.val();
  size = buffer_size_.val();
  if (ioctl_buffer_.isNull()) {
    buffer = nullptr;
  } else {
    auto dataBuf = reinterpret_cast<const char *>(ioctl_buffer_.val());
    buffer = new char[size];
    std::memcpy(buffer, dataBuf, replayed_ret_val_);
  }
  SystemCallTraceReplayModule::prepareRow();
}
