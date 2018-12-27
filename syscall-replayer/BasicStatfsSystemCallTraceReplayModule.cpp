/*
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * BasicStatfsSystemCallTraceReplayModule header file.
 *
 * Read BasicStatfsSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "BasicStatfsSystemCallTraceReplayModule.hpp"

typedef unsigned long u_long;

BasicStatfsSystemCallTraceReplayModule::BasicStatfsSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      verify_(verify_flag),
      statfs_result_type_(series, "statfs_result_type", Field::flag_nullable),
      statfs_result_bsize_(series, "statfs_result_bsize", Field::flag_nullable),
      statfs_result_blocks_(series, "statfs_result_blocks",
                            Field::flag_nullable),
      statfs_result_bfree_(series, "statfs_result_bfree", Field::flag_nullable),
      statfs_result_bavail_(series, "statfs_result_bavail",
                            Field::flag_nullable),
      statfs_result_files_(series, "statfs_result_files", Field::flag_nullable),
      statfs_result_ffree_(series, "statfs_result_ffree", Field::flag_nullable),
      statfs_result_namelen_(series, "statfs_result_namelen",
                             Field::flag_nullable),
      statfs_result_frsize_(series, "statfs_result_frsize",
                            Field::flag_nullable),
      statfs_result_flags_(series, "statfs_result_flags",
                           Field::flag_nullable) {}

void BasicStatfsSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info(
      "file system type(", statfs_result_type_.val(), "), ", "block size(",
      statfs_result_bsize_.val(), "), ", "total blocks(",
      statfs_result_blocks_.val(), "), ", "free blocks(",
      statfs_result_bfree_.val(), "), ", "available blocks(",
      statfs_result_bavail_.val(), "), ", "total file nodes(",
      statfs_result_files_.val(), "), ", "free file nodes(",
      statfs_result_ffree_.val(), "), ", "Maximum namelength(",
      statfs_result_namelen_.val(), ") ", "fragment size(",
      statfs_result_frsize_.val(), ") ", "mount flags(",
      statfs_result_flags_.val(), ")");
}

void BasicStatfsSystemCallTraceReplayModule::verifyResult(
    struct statfs replayed_statfs_buf) {
  u_int statfs_result_type = (u_int)statfs_result_type_.val();
  u_int statfs_result_bsize = (u_int)statfs_result_bsize_.val();
  u_long statfs_result_blocks = (u_long)statfs_result_blocks_.val();
  u_long statfs_result_bfree = (u_long)statfs_result_bfree_.val();
  u_long statfs_result_bavail = (u_long)statfs_result_bavail_.val();
  u_long statfs_result_files = (u_long)statfs_result_files_.val();
  u_long statfs_result_ffree = (u_long)statfs_result_ffree_.val();
  u_long statfs_result_namelen = (u_long)statfs_result_namelen_.val();
  u_long statfs_result_frsize = (u_long)statfs_result_frsize_.val();
  u_long statfs_result_flags = (u_long)statfs_result_flags_.val();

  // Verify statfs buffer contents in the trace file are same
  if (statfs_result_type != replayed_statfs_buf.f_type ||
      statfs_result_bsize != replayed_statfs_buf.f_bsize ||
      statfs_result_blocks != replayed_statfs_buf.f_blocks ||
      statfs_result_bfree != replayed_statfs_buf.f_bfree ||
      statfs_result_bavail != replayed_statfs_buf.f_bavail ||
      statfs_result_files != replayed_statfs_buf.f_files ||
      statfs_result_ffree != replayed_statfs_buf.f_ffree ||
      statfs_result_namelen != (u_long)replayed_statfs_buf.f_namelen ||
      statfs_result_frsize != (u_long)replayed_statfs_buf.f_frsize ||
      statfs_result_flags != (u_long)replayed_statfs_buf.f_flags) {
    // Statfs buffers aren't same
    syscall_logger_->log_err("Verification of ", sys_call_name_,
                             " buffer content failed.");
    if (!default_mode()) {
      syscall_logger_->log_warn(
          "time called:",
          boost::format(DEC_PRECISION) % Tfrac_to_sec(time_called()),
          "Captured ", sys_call_name_, " content is different from replayed ",
          sys_call_name_, " content");
      syscall_logger_->log_warn(
          "Captured file system type: ", statfs_result_type, ", ",
          "Replayed file system type: ", replayed_statfs_buf.f_type);
      syscall_logger_->log_warn(
          "Captured block size: ", statfs_result_bsize,
          ", Replayed block size: ", replayed_statfs_buf.f_bsize);
      syscall_logger_->log_warn(
          "Captured total blocks: ", statfs_result_blocks,
          ", Replayed total blocks: ", replayed_statfs_buf.f_blocks);
      syscall_logger_->log_warn(
          "Captured free blocks: ", statfs_result_bfree,
          ", Replayed free blocks: ", replayed_statfs_buf.f_bfree);
      syscall_logger_->log_warn(
          "Captured available blocks: ", statfs_result_bavail,
          ", Replayed available blocks: ", replayed_statfs_buf.f_bavail);
      syscall_logger_->log_warn(
          "Captured total file inodes: ", statfs_result_files,
          ", Replayed total file inodes: ", replayed_statfs_buf.f_files);
      syscall_logger_->log_warn(
          "Captured free file nodes: ", statfs_result_ffree,
          ", Replayed free file nodes: ", replayed_statfs_buf.f_ffree);
      syscall_logger_->log_warn(
          "Captured maximum namelength: ", statfs_result_namelen,
          ", Replayed maximum namelength: ", replayed_statfs_buf.f_namelen);
      syscall_logger_->log_warn(
          "Captured fragment size: ", statfs_result_frsize,
          ", Replayed fragment size: ", replayed_statfs_buf.f_frsize);
      syscall_logger_->log_warn(
          "Captured mount flags: ", statfs_result_flags,
          ", Replayed mount flags: ", replayed_statfs_buf.f_flags);
      if (abort_mode()) {
        abort();
      }
    }
  } else {
    if (verbose_mode()) {
      syscall_logger_->log_info("Verification of ", sys_call_name_,
                                " buffer succeeded.");
    }
  }
}

StatfsSystemCallTraceReplayModule::StatfsSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : BasicStatfsSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
                                             warn_level_flag),
      given_pathname_(series, "given_pathname") {
  sys_call_name_ = "statfs";
}

void StatfsSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", given_pathname_.val(), "),");
  BasicStatfsSystemCallTraceReplayModule::print_specific_fields();
}

void StatfsSystemCallTraceReplayModule::processRow() {
  struct statfs statfs_buf;
  const char *pathname = reinterpret_cast<const char *>(given_pathname_.val());

  // replay the statfs system call
  replayed_ret_val_ = statfs(pathname, &statfs_buf);

  if (verify_) {
    BasicStatfsSystemCallTraceReplayModule::verifyResult(statfs_buf);
  }
}

FStatfsSystemCallTraceReplayModule::FStatfsSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : BasicStatfsSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
                                             warn_level_flag),
      descriptor_(series, "descriptor") {
  sys_call_name_ = "fstatfs";
}

void FStatfsSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info("traced fd(", descriptor_.val(), "), ",
                            "replayed fd(", replayed_fd, "), ");
  BasicStatfsSystemCallTraceReplayModule::print_specific_fields();
}

void FStatfsSystemCallTraceReplayModule::processRow() {
  struct statfs statfs_buf;
  pid_t pid = executing_pid();
  int fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());

  if (fd == SYSCALL_SIMULATED) {
    /*
     * FD for the fstatfs system call originated from a socket().
     * The system call will not be replayed.
     * Traced return value will be returned.
     */
    replayed_ret_val_ = return_value_.val();
    return;
  }
  // replay the fstatfs system call
  replayed_ret_val_ = fstatfs(fd, &statfs_buf);

  if (verify_) {
    BasicStatfsSystemCallTraceReplayModule::verifyResult(statfs_buf);
  }
}
