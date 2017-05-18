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
 * GetdentsSystemCallTraceReplayModule header file
 *
 * Read GetdentsSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "GetdentsSystemCallTraceReplayModule.hpp"

GetdentsSystemCallTraceReplayModule::
GetdentsSystemCallTraceReplayModule(DataSeriesModule &source,
				    bool verbose_flag,
				    bool verify_flag,
				    int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  verify_(verify_flag),
  descriptor_(series, "descriptor"),
  dirent_buffer_(series, "dirent_buffer", Field::flag_nullable),
  count_(series, "count") {
  sys_call_name_ = "getdents";
}

void GetdentsSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info("traced fd(", descriptor_.val(), "), ",
    "replayed fd(", replayed_fd, "), "
    "count(", count_.val(), ")");
}

void GetdentsSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  int count = count_.val();
  struct dirent *buffer = (struct dirent *) malloc(count);
  replayed_ret_val_ = syscall(SYS_getdents, fd, buffer, count);

  if (verify_ == true) {
    // Verify dirent buffer data and data in the trace file are same
    if (memcmp(dirent_buffer_.val(), buffer, replayed_ret_val_) != 0) {
      // Data aren't same
      syscall_logger_->log_err("Verification of data in getdents failed.");
      if (!default_mode()) {
        syscall_logger_->log_warn("time called: ", \
		      boost::format(DEC_PRECISION) % Tfrac_to_sec(time_called()), \
		      "Captured getdents data is different from replayed ", \
		      "getdents data.");
        if (abort_mode()) {
          abort();
        }
      }
    } else {
      if (verbose_mode()) {
        syscall_logger_->log_info("Verification of data in getdents success.");
      }
    }
  }
  free(buffer);
}
