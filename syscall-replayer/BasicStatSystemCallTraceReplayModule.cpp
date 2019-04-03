/*
 * Copyright (c) 2016 Nina Brown
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
 * BasicStatSystemCallTraceReplayModule header file.
 *
 * Read BasicStatSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "BasicStatSystemCallTraceReplayModule.hpp"
#include <cstring>
#include <memory>

BasicStatSystemCallTraceReplayModule::BasicStatSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      verify_(verify_flag),
      stat_result_dev_(series, "stat_result_dev", Field::flag_nullable),
      stat_result_ino_(series, "stat_result_ino", Field::flag_nullable),
      stat_result_mode_(series, "stat_result_mode", Field::flag_nullable),
      stat_result_nlink_(series, "stat_result_nlink", Field::flag_nullable),
      stat_result_uid_(series, "stat_result_uid", Field::flag_nullable),
      stat_result_gid_(series, "stat_result_gid", Field::flag_nullable),
      stat_result_rdev_(series, "stat_result_rdev", Field::flag_nullable),
      stat_result_blksize_(series, "stat_result_blksize", Field::flag_nullable),
      stat_result_blocks_(series, "stat_result_blocks", Field::flag_nullable),
      stat_result_size_(series, "stat_result_size", Field::flag_nullable),
      stat_result_atime_(series, "stat_result_atime", Field::flag_nullable),
      stat_result_mtime_(series, "stat_result_mtime", Field::flag_nullable),
      stat_result_ctime_(series, "stat_result_ctime", Field::flag_nullable) {}

int BasicStatSystemCallTraceReplayModule::print_mode_value(u_int st_mode) {
  int printable_mode = 0;
  mode_t mode = (mode_t)st_mode;
  if ((mode & S_ISUID) != 0u) {
    printable_mode |= 0x4000;
  }
  if ((mode & S_ISGID) != 0u) {
    printable_mode |= 0x2000;
  }
  if ((mode & S_ISVTX) != 0u) {
    printable_mode |= 0x1000;
  }
  if ((mode & S_IRUSR) != 0u) {
    printable_mode |= 0x400;
  }
  if ((mode & S_IWUSR) != 0u) {
    printable_mode |= 0x200;
  }
  if ((mode & S_IXUSR) != 0u) {
    printable_mode |= 0x100;
  }
  if ((mode & S_IRGRP) != 0u) {
    printable_mode |= 0x040;
  }
  if ((mode & S_IWGRP) != 0u) {
    printable_mode |= 0x020;
  }
  if ((mode & S_IXGRP) != 0u) {
    printable_mode |= 0x010;
  }
  if ((mode & S_IROTH) != 0u) {
    printable_mode |= 0x0004;
  }
  if ((mode & S_IWOTH) != 0u) {
    printable_mode |= 0x002;
  }
  if ((mode & S_IXOTH) != 0u) {
    printable_mode |= 0x001;
  }

  return printable_mode;
}

void BasicStatSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info(
      "device id(", statDev, "), ", "file inode number(", statINo, "), ",
      "file mode(", print_mode_value(statMode), "), ", "file nlinks(",
      statNLink, "), ", "file UID(", statUID, "), ", "file GID(", statGID,
      "), ", "file size(", statSize, "), ", "file blksize(", statBlkSize, "), ",
      "file blocks(", statBlocks, ") ", "file atime(",
      boost::format(DEC_PRECISION) % Tfrac_to_sec(statATime), ") ",
      "file mtime(", boost::format(DEC_PRECISION) % Tfrac_to_sec(statMTime),
      ") ", "file ctime(",
      boost::format(DEC_PRECISION) % Tfrac_to_sec(statCTime), ")");
}

void BasicStatSystemCallTraceReplayModule::copyStatStruct(
    uint32_t dev, uint32_t ino, uint32_t mode, uint32_t nlink, uint32_t uid,
    uint32_t gid, uint32_t rdev, uint32_t blksize, uint32_t blocks,
    int64_t size, uint64_t atime, uint64_t mtime, uint64_t ctime) {
  statDev = dev;
  statINo = ino;
  statMode = mode;
  statNLink = nlink;
  statUID = uid;
  statGID = gid;
  statRDev = rdev;
  statBlkSize = blksize;
  statBlocks = blocks;
  statSize = size;
  statATime = atime;
  statMTime = mtime;
  statCTime = ctime;
}

void BasicStatSystemCallTraceReplayModule::prepareRow() {
  statDev = stat_result_dev_.val();
  statINo = stat_result_ino_.val();
  statMode = stat_result_mode_.val();
  statNLink = stat_result_nlink_.val();
  statUID = stat_result_uid_.val();
  statGID = stat_result_gid_.val();
  statRDev = stat_result_rdev_.val();
  statBlkSize = stat_result_blksize_.val();
  statBlocks = stat_result_blocks_.val();
  statSize = stat_result_size_.val();
  statATime = stat_result_atime_.val();
  statMTime = stat_result_mtime_.val();
  statCTime = stat_result_ctime_.val();
  SystemCallTraceReplayModule::prepareRow();
}

void BasicStatSystemCallTraceReplayModule::verifyResult(
    struct stat replayed_stat_buf) {
  /*
   * Verify stat buffer contents in the trace file are same
   * We are comparing only key fields captured in strace : st_ino,
   * st_mode, st_nlink, st_uid, st_gid, st_size, st_blksize and
   * st_blocks fileds of struct stat.
   */
  /*
   * Fixed: Inode numbers can not be matched if we are creating
   * new files even they have the same path
   */
  if (statMode != replayed_stat_buf.st_mode ||
      statNLink != replayed_stat_buf.st_nlink ||
      statUID != replayed_stat_buf.st_uid ||
      statGID != replayed_stat_buf.st_gid ||
      statSize != replayed_stat_buf.st_size ||
      statBlkSize != replayed_stat_buf.st_blksize ||
      statBlocks != replayed_stat_buf.st_blocks) {
    // Stat buffers aren't same
    syscall_logger_->log_err("Verification of ", sys_call_name_,
                             " buffer content failed.");
    if (!default_mode()) {
      syscall_logger_->log_warn(
          "time called:",
          boost::format(DEC_PRECISION) % Tfrac_to_sec(time_called()),
          " Captured ", sys_call_name_, " content is different from replayed ",
          sys_call_name_, " content");
      syscall_logger_->log_warn(
          "Captured file inode: ", statINo, ", ",
          "Replayed file inode: ", replayed_stat_buf.st_ino);
      syscall_logger_->log_warn(
          "Captured file mode: ", print_mode_value(statMode), ", ",
          "Replayed file mode: ", print_mode_value(replayed_stat_buf.st_mode));
      syscall_logger_->log_warn(
          "Captured file nlink: ", statNLink, ", ",
          "Replayed file nlink: ", replayed_stat_buf.st_nlink);
      syscall_logger_->log_warn(
          "Captured file UID: ", statUID, ", ",
          "Replayed file UID: ", replayed_stat_buf.st_uid);
      syscall_logger_->log_warn(
          "Captured file GID: ", statGID, ", ",
          "Replayed file GID: ", replayed_stat_buf.st_gid);
      syscall_logger_->log_warn(
          "Captured file size: ", statSize, ", ",
          "Replayed file size: ", replayed_stat_buf.st_size);
      syscall_logger_->log_warn(
          "Captured file blksize: ", statBlkSize, ", ",
          "Replayed file blksize: ", replayed_stat_buf.st_blksize);
      syscall_logger_->log_warn(
          "Captured file blocks: ", statBlocks, ", ",
          "Replayed file blocks: ", replayed_stat_buf.st_blocks);

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

StatSystemCallTraceReplayModule::StatSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : BasicStatSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
                                           warn_level_flag),
      given_pathname_(series, "given_pathname", Field::flag_nullable) {
  sys_call_name_ = "stat";
}

void StatSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", pathname, "),");
  BasicStatSystemCallTraceReplayModule::print_specific_fields();
}

void StatSystemCallTraceReplayModule::processRow() {
  struct stat stat_buf;

  // replay the stat system call
  replayed_ret_val_ = stat(pathname, &stat_buf);

  if (verify_) {
    BasicStatSystemCallTraceReplayModule::verifyResult(stat_buf);
  }

  delete[] pathname;
}

void StatSystemCallTraceReplayModule::prepareRow() {
  auto pathBuf = reinterpret_cast<const char *>(given_pathname_.val());
  pathname = new char[std::strlen(pathBuf) + 1];
  std::strncpy(pathname, pathBuf, (std::strlen(pathBuf) + 1));
  BasicStatSystemCallTraceReplayModule::prepareRow();
}

LStatSystemCallTraceReplayModule::LStatSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : BasicStatSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
                                           warn_level_flag),
      given_pathname_(series, "given_pathname", Field::flag_nullable) {
  sys_call_name_ = "lstat";
}

void LStatSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", pathname, "),");
  BasicStatSystemCallTraceReplayModule::print_specific_fields();
}

void LStatSystemCallTraceReplayModule::processRow() {
  struct stat stat_buf;
  // replay the lstat system call
  replayed_ret_val_ = lstat(pathname, &stat_buf);

  if (verify_) {
    BasicStatSystemCallTraceReplayModule::verifyResult(stat_buf);
  }
}

void LStatSystemCallTraceReplayModule::prepareRow() {
  auto pathBuf = reinterpret_cast<const char *>(given_pathname_.val());
  pathname = new char[std::strlen(pathBuf) + 1];
  std::strncpy(pathname, pathBuf, (std::strlen(pathBuf) + 1));
  BasicStatSystemCallTraceReplayModule::prepareRow();
}

FStatSystemCallTraceReplayModule::FStatSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : BasicStatSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
                                           warn_level_flag),
      descriptor_(series, "descriptor") {
  sys_call_name_ = "fstat";
}

void FStatSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptorVal);
  syscall_logger_->log_info("traced fd(", descriptorVal, "), ", "replayed fd(",
                            replayed_fd, "), ");
  BasicStatSystemCallTraceReplayModule::print_specific_fields();
}

void FStatSystemCallTraceReplayModule::processRow() {
  struct stat stat_buf;
  int fd = replayer_resources_manager_.get_fd(executingPidVal, descriptorVal);
  if (fd == SYSCALL_SIMULATED) {
    /*
     * FD for the fstat system call originated from a socket().
     * The system call will not be replayed.
     * Original return value will be returned.
     */
    return;
  }

  // replay the fstat system call
  replayed_ret_val_ = fstat(fd, &stat_buf);

  if (verify_) {
    BasicStatSystemCallTraceReplayModule::verifyResult(stat_buf);
  }
}

void FStatSystemCallTraceReplayModule::prepareRow() {
  descriptorVal = descriptor_.val();
  replayed_ret_val_ = return_value_.val();
  BasicStatSystemCallTraceReplayModule::prepareRow();
}

FStatatSystemCallTraceReplayModule::FStatatSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, bool verify_flag,
    int warn_level_flag)
    : BasicStatSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
                                           warn_level_flag),
      descriptor_(series, "descriptor"),
      given_pathname_(series, "given_pathname", Field::flag_nullable),
      flags_value_(series, "flags_value", Field::flag_nullable) {
  sys_call_name_ = "fstatat";
}

void FStatatSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, traced_fd);
  syscall_logger_->log_info("traced fd(", traced_fd, "), ", "replayed fd(",
                            replayed_fd, ")", "pathname(", pathname, "), ",
                            "flags_value(", flag_value, ")");
  BasicStatSystemCallTraceReplayModule::print_specific_fields();
}

void FStatatSystemCallTraceReplayModule::processRow() {
  struct stat stat_buf;
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, traced_fd);

  if (replayed_fd == SYSCALL_SIMULATED) {
    /*
     * FD for the fstatat system call originated from a socket().
     * The system call will not be replayed.
     * Original return value will be returned.
     */
    replayed_ret_val_ = return_value();
    return;
  }
  // replay the fstat system call
  replayed_ret_val_ = fstatat(replayed_fd, pathname, &stat_buf, flag_value);

  if (verify_) {
    BasicStatSystemCallTraceReplayModule::verifyResult(stat_buf);
  }
  delete[] pathname;
}

void FStatatSystemCallTraceReplayModule::prepareRow() {
  traced_fd = descriptor_.val();
  flag_value = flags_value_.val();
  auto pathBuf = reinterpret_cast<const char *>(given_pathname_.val());
  pathname = new char[std::strlen(pathBuf) + 1];
  std::strncpy(pathname, pathBuf, std::strlen(pathBuf) + 1);
  BasicStatSystemCallTraceReplayModule::prepareRow();
}
