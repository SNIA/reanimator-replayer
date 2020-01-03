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
      "file system type(", statfsType, "), ", "block size(", statfsBsize, "), ",
      "total blocks(", statfsBlocks, "), ", "free blocks(", statfsBfree, "), ",
      "available blocks(", statfsBavail, "), ", "total file nodes(",
      statfsFiles, "), ", "free file nodes(", statfsFFree, "), ",
      "Maximum namelength(", statfsNamelen, ") ", "fragment size(",
      statfsFrsize, ") ", "mount flags(", statfsFlags, ")");
}

void BasicStatfsSystemCallTraceReplayModule::verifyResult(
    struct statfs replayed_statfs_buf) {
  // Verify statfs buffer contents in the trace file are same
  if (statfsType != replayed_statfs_buf.f_type ||
      statfsBsize != replayed_statfs_buf.f_bsize ||
      statfsBlocks != replayed_statfs_buf.f_blocks ||
      statfsBfree != replayed_statfs_buf.f_bfree ||
      statfsBavail != replayed_statfs_buf.f_bavail ||
      statfsFiles != replayed_statfs_buf.f_files ||
      statfsFFree != replayed_statfs_buf.f_ffree ||
      statfsNamelen != (u_long)replayed_statfs_buf.f_namelen ||
      statfsFrsize != (u_long)replayed_statfs_buf.f_frsize ||
      statfsFlags != (u_long)replayed_statfs_buf.f_flags) {
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
          "Captured file system type: ", statfsType, ", ",
          "Replayed file system type: ", replayed_statfs_buf.f_type);
      syscall_logger_->log_warn(
          "Captured block size: ", statfsBsize,
          ", Replayed block size: ", replayed_statfs_buf.f_bsize);
      syscall_logger_->log_warn(
          "Captured total blocks: ", statfsBlocks,
          ", Replayed total blocks: ", replayed_statfs_buf.f_blocks);
      syscall_logger_->log_warn(
          "Captured free blocks: ", statfsBfree,
          ", Replayed free blocks: ", replayed_statfs_buf.f_bfree);
      syscall_logger_->log_warn(
          "Captured available blocks: ", statfsBavail,
          ", Replayed available blocks: ", replayed_statfs_buf.f_bavail);
      syscall_logger_->log_warn(
          "Captured total file inodes: ", statfsFiles,
          ", Replayed total file inodes: ", replayed_statfs_buf.f_files);
      syscall_logger_->log_warn(
          "Captured free file nodes: ", statfsFFree,
          ", Replayed free file nodes: ", replayed_statfs_buf.f_ffree);
      syscall_logger_->log_warn(
          "Captured maximum namelength: ", statfsNamelen,
          ", Replayed maximum namelength: ", replayed_statfs_buf.f_namelen);
      syscall_logger_->log_warn(
          "Captured fragment size: ", statfsFrsize,
          ", Replayed fragment size: ", replayed_statfs_buf.f_frsize);
      syscall_logger_->log_warn(
          "Captured mount flags: ", statfsFlags,
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

void BasicStatfsSystemCallTraceReplayModule::copyStatfsStruct(
    uint32_t type, uint32_t bsize, uint64_t blocks, uint64_t bfree,
    uint64_t bavail, uint64_t files, uint64_t ffree, uint64_t namelen,
    uint64_t frsize, uint64_t flags) {
  statfsType = type;
  statfsBsize = bsize;
  statfsBlocks = blocks;
  statfsBfree = bfree;
  statfsBavail = bavail;
  statfsFiles = files;
  statfsFFree = ffree;
  statfsNamelen = namelen;
  statfsFrsize = frsize;
  statfsFlags = flags;
}

void BasicStatfsSystemCallTraceReplayModule::prepareRow() {
  statfsType = static_cast<uint32_t>(statfs_result_type_.val());
  statfsBsize = static_cast<uint32_t>(statfs_result_bsize_.val());
  statfsBlocks = static_cast<uint64_t>(statfs_result_blocks_.val());
  statfsBfree = static_cast<uint64_t>(statfs_result_bfree_.val());
  statfsBavail = static_cast<uint64_t>(statfs_result_bavail_.val());
  statfsFiles = static_cast<uint64_t>(statfs_result_files_.val());
  statfsFFree = static_cast<uint64_t>(statfs_result_ffree_.val());
  statfsNamelen = static_cast<uint64_t>(statfs_result_namelen_.val());
  statfsFrsize = static_cast<uint64_t>(statfs_result_frsize_.val());
  statfsFlags = static_cast<uint64_t>(statfs_result_flags_.val());
  SystemCallTraceReplayModule::prepareRow();
}

StatfsSystemCallTraceReplayModule::StatfsSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : BasicStatfsSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
                                             warn_level_flag),
      given_pathname_(series, "given_pathname", Field::flag_nullable) {
  sys_call_name_ = "statfs";
}

void StatfsSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", pathname, "),");
  BasicStatfsSystemCallTraceReplayModule::print_specific_fields();
}

void StatfsSystemCallTraceReplayModule::processRow() {
  struct statfs statfs_buf;
  // replay the statfs system call
  replayed_ret_val_ = statfs(pathname, &statfs_buf);

  if (verify_) {
    BasicStatfsSystemCallTraceReplayModule::verifyResult(statfs_buf);
  }
  delete[] pathname;
}

void StatfsSystemCallTraceReplayModule::prepareRow() {
  auto pathBuf = reinterpret_cast<const char *>(given_pathname_.val());
  pathname = copyPath(pathBuf);
  BasicStatfsSystemCallTraceReplayModule::prepareRow();
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
