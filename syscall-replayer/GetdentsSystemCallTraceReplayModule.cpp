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

#define getdents(fd, buffer, count) syscall(SYS_getdents, fd, buffer, count)

GetdentsSystemCallTraceReplayModule::GetdentsSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      verify_(verify_flag),
      descriptor_(series, "descriptor"),
      dirent_buffer_(series, "dirent_buffer", Field::flag_nullable),
      count_(series, "count") {
  sys_call_name_ = "getdents";
}

void GetdentsSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, traced_fd);
  syscall_logger_->log_info("traced fd(", traced_fd, "), ", "replayed fd(",
                            replayed_fd,
                            "), "
                            "count(",
                            count_val, ")");
}

bool GetdentsSystemCallTraceReplayModule::compareResults(
    struct dirent *realExecution, int64_t sizeOfRealExecution,
    struct dirent *replayed, int64_t sizeOfReplay) {
  if (sizeOfRealExecution != sizeOfReplay || sizeOfRealExecution == 0) {
    return false;
  }
  for (int pos = 0; pos < return_val;) {
    // getting dirent data for the real execution buffer
    auto direntRecOrg =
        (struct dirent *)(reinterpret_cast<char *>(realExecution) + pos);
    auto typeOrg = *((reinterpret_cast<char *>(realExecution)) + pos +
                     direntRecOrg->d_reclen - 1);
    auto typeStrOrg = getDirentTypeStr(typeOrg);
    auto offsetOrg = direntRecOrg->d_off;
    auto recordLenOrg = direntRecOrg->d_reclen;
    auto direntNameOrg = direntRecOrg->d_name;

    // getting dirent data for the replayed execution buffer
    auto direntRecReplayed =
        (struct dirent *)(reinterpret_cast<char *>(replayed) + pos);
    auto typeReplayed = *((reinterpret_cast<char *>(replayed)) + pos +
                          direntRecReplayed->d_reclen - 1);
    auto typeStrReplayed = getDirentTypeStr(typeReplayed);
    auto offsetReplayed = direntRecReplayed->d_off;
    auto recordLenReplayed = direntRecReplayed->d_reclen;
    auto direntNameReplayed = direntRecReplayed->d_name;

    if (typeStrOrg != typeStrReplayed || offsetOrg != offsetReplayed ||
        recordLenOrg != recordLenReplayed ||
        direntNameOrg != direntNameReplayed) {
      return false;
    }
    pos += recordLenOrg;
  }
  return true;
}

void GetdentsSystemCallTraceReplayModule::processRow() {
  // Get replaying file descriptor.
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, traced_fd);
  int count = count_val;

  if (fd == SYSCALL_SIMULATED) {
    /*
     * FD for the getdents system call originated from a socket().
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_val;
    return;
  }
  struct dirent *buffer = (struct dirent *)new char[count];
  if (buffer == nullptr) {
    replayed_ret_val_ = ENOMEM;
  } else {
    replayed_ret_val_ = (int64_t)getdents(fd, buffer, count);
  }

  if (verify_) {
    // Verify dirent buffer data and data in the trace file are same
    if (compareResults(dirent_buffer_val, return_val, buffer,
                       replayed_ret_val_)) {
      // Data aren't same
      syscall_logger_->log_err("Verification of data in getdents failed.");
      if (!default_mode()) {
        syscall_logger_->log_warn(
            "time called: ",
            boost::format(DEC_PRECISION) % Tfrac_to_sec(time_called()),
            " captured getdents data is different from replayed ",
            "getdents data.");
        syscall_logger_->log_warn("replayed ret val ", replayed_ret_val_,
                                  " ret val ", return_val);

        syscall_logger_->log_warn("Captured dirents");
        printDirents(dirent_buffer_val);
        printDirents(buffer);
        if (abort_mode()) {
          abort();
        }
      }
    } else {
      if (verbose_mode()) {
        syscall_logger_->log_info("Verification of data in getdents success.");
      }
    }
    delete[] dirent_buffer_val;
  }
  if (buffer != nullptr) {
    delete[] buffer;
  }
}

void GetdentsSystemCallTraceReplayModule::prepareRow() {
  traced_fd = (unsigned int)descriptor_.val();
  count_val = (unsigned int)count_.val();
  return_val = (int64_t)return_value_.val();

  if (verify_) {
    auto dataBuf = reinterpret_cast<const char *>(dirent_buffer_.val());
    dirent_buffer_val = (struct dirent *)new char[count_val];
    std::memcpy(dirent_buffer_val, dataBuf, count_val);
  }
  SystemCallTraceReplayModule::prepareRow();
}
