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
 * This file implements all the functions in the
 * IoctlSystemCallTraceReplayModule header file
 *
 * Read IoctlSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "IoctlSystemCallTraceReplayModule.hpp"

IoctlSystemCallTraceReplayModule::
IoctlSystemCallTraceReplayModule(DataSeriesModule &source,
                                 bool verbose_flag,
                                 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  descriptor_(series, "descriptor"),
  request_(series, "request"),
  parameter_(series, "parameter", Field::flag_nullable),
  ioctl_buffer_(series, "ioctl_buffer", Field::flag_nullable),
  buffer_size_(series, "buffer_size") {
  sys_call_name_ = "ioctl";
}

void IoctlSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "descriptor(" << descriptor_.val() << "), ";
  std::cout << "request(" << std::hex << request_.val() << std::dec << ")";
}

void IoctlSystemCallTraceReplayModule::processRow() {
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];
  u_long request = request_.val();
  void *buf = malloc(buffer_size_.val());
  memcpy(buf, ioctl_buffer_.val(), buffer_size_.val());

  // Replay the ioctl system call
  replayed_ret_val_ = ioctl(fd, request, buf);

  free(buf);
}
