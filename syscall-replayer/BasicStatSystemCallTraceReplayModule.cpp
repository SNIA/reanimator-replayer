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

BasicStatSystemCallTraceReplayModule::
BasicStatSystemCallTraceReplayModule(DataSeriesModule &source,
				     bool verbose_flag,
				     bool verify_flag,
				     int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
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
  stat_result_ctime_(series, "stat_result_ctime", Field::flag_nullable) { }

int BasicStatSystemCallTraceReplayModule::print_mode_value(u_int st_mode) {
  int printable_mode = 0;
  mode_t mode = (mode_t) st_mode;
  if (mode & S_ISUID)
    printable_mode |= 0x4000;
  if (mode & S_ISGID)
    printable_mode |= 0x2000;
  if (mode & S_ISVTX)
    printable_mode |= 0x1000;
  if (mode & S_IRUSR)
    printable_mode |= 0x400;
  if (mode & S_IWUSR)
    printable_mode |= 0x200;
  if (mode & S_IXUSR)
    printable_mode |= 0x100;
  if (mode & S_IRGRP)
    printable_mode |= 0x040;
  if (mode & S_IWGRP)
    printable_mode |= 0x020;
  if (mode & S_IXGRP)
    printable_mode |= 0x010;
  if (mode & S_IROTH)
    printable_mode |= 0x0004;
  if (mode & S_IWOTH)
    printable_mode |= 0x002;
  if (mode & S_IXOTH)
    printable_mode |= 0x001;

  return printable_mode;
}

void BasicStatSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("device id(", stat_result_dev_.val(), "), ", \
  	"file inode number(", stat_result_ino_.val(), "), ", "file mode(", \
  	boost::format("0x%02x") % print_mode_value(stat_result_mode_.val()), "), ", \
  	"file nlinks(", stat_result_nlink_.val(), "), ", \
  	"file UID(", stat_result_uid_.val(), "), ", \
  	"file GID(", stat_result_gid_.val(), "), ", \
  	"file size(", stat_result_size_.val(), "), ", \
  	"file blksize(", stat_result_blksize_.val(), "), ", \
  	"file blocks(", stat_result_blocks_.val(), ") ", \
  	"file atime(", boost::format(DEC_PRECISION) % Tfrac_to_sec(stat_result_atime_.val()), ") ", \
  	"file mtime(", boost::format(DEC_PRECISION) % Tfrac_to_sec(stat_result_mtime_.val()), ") ", \
  	"file ctime(", boost::format(DEC_PRECISION) % Tfrac_to_sec(stat_result_ctime_.val()), ")");
}

void BasicStatSystemCallTraceReplayModule::verifyResult(
					      struct stat replayed_stat_buf) {
  u_int stat_result_ino = (u_int) stat_result_ino_.val();
  u_int stat_result_mode = stat_result_mode_.val();
  u_int stat_result_nlink = (u_int) stat_result_nlink_.val();
  u_int stat_result_uid = (u_int) stat_result_uid_.val();
  u_int stat_result_gid = (u_int) stat_result_gid_.val();
  long stat_result_size = stat_result_size_.val();
  int stat_result_blksize = (int) stat_result_blksize_.val();
  int stat_result_blocks = (int) stat_result_blocks_.val();

  /*
   * Verify stat buffer contents in the trace file are same
   * We are comparing only key fields captured in strace : st_ino,
   * st_mode, st_nlink, st_uid, st_gid, st_size, st_blksize and
   * st_blocks fileds of struct stat.
   */
  if (stat_result_ino != replayed_stat_buf.st_ino ||
      stat_result_mode != replayed_stat_buf.st_mode ||
      stat_result_nlink != replayed_stat_buf.st_nlink ||
      stat_result_uid != replayed_stat_buf.st_uid ||
      stat_result_gid != replayed_stat_buf.st_gid ||
      stat_result_size != replayed_stat_buf.st_size ||
      stat_result_blksize != replayed_stat_buf.st_blksize ||
      stat_result_blocks != replayed_stat_buf.st_blocks) {

      // Stat buffers aren't same
      syscall_logger_->log_err("Verification of ", sys_call_name_, \
	      " buffer content failed.");
      if (!default_mode()) {
        syscall_logger_->log_warn("time called:", \
          boost::format(DEC_PRECISION) % Tfrac_to_sec(time_called()), \
          " Captured ", sys_call_name_, \
          " content is different from replayed ", \
          sys_call_name_, " content");
        syscall_logger_->log_warn("Captured file inode: ", stat_result_ino, ", ", \
          "Replayed file inode: ", replayed_stat_buf.st_ino);
        syscall_logger_->log_warn("Captured file mode: ", \
          boost::format("0x%02x") % print_mode_value(stat_result_mode), ", ", \
          "Replayed file mode: ", \
          boost::format("0x%02x") % print_mode_value(replayed_stat_buf.st_mode));
      	syscall_logger_->log_warn("Captured file nlink: ", stat_result_nlink, ", ", \
          "Replayed file nlink: ", replayed_stat_buf.st_nlink);
      	syscall_logger_->log_warn("Captured file UID: ", stat_result_uid, ", ", \
          "Replayed file UID: ", replayed_stat_buf.st_uid);
      	syscall_logger_->log_warn("Captured file GID: ", stat_result_gid, ", ", \
          "Replayed file GID: ", replayed_stat_buf.st_gid);
      	syscall_logger_->log_warn("Captured file size: ", stat_result_size, ", ", \
          "Replayed file size: ", replayed_stat_buf.st_size);
      	syscall_logger_->log_warn("Captured file blksize: ", stat_result_blksize, ", ", \
          "Replayed file blksize: ", replayed_stat_buf.st_blksize);
      	syscall_logger_->log_warn("Captured file blocks: ", stat_result_blocks, ", ", \
          "Replayed file blocks: ", replayed_stat_buf.st_blocks);

        if (abort_mode()) {
          abort();
        }
      }
    } else {
      if (verbose_mode()) {
        syscall_logger_->log_info("Verification of ", sys_call_name_, \
          " buffer succeeded.");
      }
  }
}

StatSystemCallTraceReplayModule::
StatSystemCallTraceReplayModule(DataSeriesModule &source,
				bool verbose_flag,
				bool verify_flag,
				int warn_level_flag):
  BasicStatSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
				       warn_level_flag),
  given_pathname_(series, "given_pathname") {
  sys_call_name_ = "stat";
}

void StatSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", given_pathname_.val(), "),");
  BasicStatSystemCallTraceReplayModule::print_specific_fields();
}

void StatSystemCallTraceReplayModule::processRow() {
  struct stat stat_buf;

  // replay the stat system call
  replayed_ret_val_ = stat(pathname, &stat_buf);

  if (verify_ == true) {
    BasicStatSystemCallTraceReplayModule::verifyResult(stat_buf);
  }
}

void StatSystemCallTraceReplayModule::prepareRow() {
  auto pathBuf = reinterpret_cast<const char *>(given_pathname_.val());
  pathname = new char[std::strlen(pathBuf)+1];
  std::strcpy(pathname, pathBuf);
}

LStatSystemCallTraceReplayModule::
LStatSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 bool verify_flag,
				 int warn_level_flag):
  BasicStatSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
				  warn_level_flag),
  given_pathname_(series, "given_pathname") {
  sys_call_name_ = "lstat";
}

void LStatSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("pathname(", given_pathname_.val(), "),");
  BasicStatSystemCallTraceReplayModule::print_specific_fields();
}

void LStatSystemCallTraceReplayModule::processRow() {
  struct stat stat_buf;
  char *pathname = (char *) given_pathname_.val();

  // replay the lstat system call
  replayed_ret_val_ = lstat(pathname, &stat_buf);

  if (verify_ == true) {
    BasicStatSystemCallTraceReplayModule::verifyResult(stat_buf);
  }
}

FStatSystemCallTraceReplayModule::
FStatSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 bool verify_flag,
				 int warn_level_flag):
  BasicStatSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
				  warn_level_flag),
  descriptor_(series, "descriptor") {
  sys_call_name_ = "fstat";
}

void FStatSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info("traced fd(", descriptor_.val(), "), ",
    "replayed fd(", replayed_fd, "), ");
  BasicStatSystemCallTraceReplayModule::print_specific_fields();
}

void FStatSystemCallTraceReplayModule::processRow() {
  struct stat stat_buf;
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

  if (verify_ == true) {
    BasicStatSystemCallTraceReplayModule::verifyResult(stat_buf);
  }
}

void FStatSystemCallTraceReplayModule::prepareRow() {
  pid = executing_pid();
  fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  replayed_ret_val_ = return_value_.val();
}

FStatatSystemCallTraceReplayModule::
FStatatSystemCallTraceReplayModule(DataSeriesModule &source,
                                   bool verbose_flag,
                                   bool verify_flag,
                                   int warn_level_flag):
  BasicStatSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
                                warn_level_flag),
  descriptor_(series, "descriptor"),
  given_pathname_(series, "given_pathname"),
  flags_value_(series, "flags_value", Field::flag_nullable) {
  sys_call_name_ = "fstatat";
}

void FStatatSystemCallTraceReplayModule::print_specific_fields() {
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  syscall_logger_->log_info("traced fd(", descriptor_.val(), "), ",
    "replayed fd(", replayed_fd, ")",
    "pathname(", given_pathname_.val(), "), ",
    "flags_value(", flags_value_.val(), ")");
  BasicStatSystemCallTraceReplayModule::print_specific_fields();
}

void FStatatSystemCallTraceReplayModule::processRow() {
  struct stat stat_buf;
  pid_t pid = executing_pid();
  int replayed_fd = replayer_resources_manager_.get_fd(pid, descriptor_.val());
  char *pathname = (char *) given_pathname_.val();

  if (replayed_fd == SYSCALL_SIMULATED) {
    /*
     * FD for the fstatat system call originated from a socket().
     * The system call will not be replayed.
     * Original return value will be returned.
     */
    replayed_ret_val_ = return_value_.val();
    return;
  }
  // replay the fstat system call
  replayed_ret_val_ = fstatat(replayed_fd, pathname, &stat_buf, flags_value_.val());

  if (verify_ == true) {
    BasicStatSystemCallTraceReplayModule::verifyResult(stat_buf);
  }
}
