/*
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
 * BasicStatfsSystemCallTraceReplayModule header file.
 *
 * Read BasicStatfsSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "BasicStatfsSystemCallTraceReplayModule.hpp"

BasicStatfsSystemCallTraceReplayModule::
BasicStatfsSystemCallTraceReplayModule(DataSeriesModule &source,
				       bool verbose_flag,
				       bool verify_flag,
				       int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  verify_(verify_flag),
  statfs_result_type_(series, "statfs_result_type", Field::flag_nullable),
  statfs_result_bsize_(series, "statfs_result_bsize", Field::flag_nullable),
  statfs_result_blocks_(series, "statfs_result_blocks", Field::flag_nullable),
  statfs_result_bfree_(series, "statfs_result_bfree", Field::flag_nullable),
  statfs_result_bavail_(series, "statfs_result_bavail", Field::flag_nullable),
  statfs_result_files_(series, "statfs_result_files", Field::flag_nullable),
  statfs_result_ffree_(series, "statfs_result_ffree", Field::flag_nullable),
  statfs_result_namelen_(series, "statfs_result_namelen", Field::flag_nullable),
  statfs_result_frsize_(series, "statfs_result_frsize", Field::flag_nullable),
  statfs_result_flags_(series, "statfs_result_flags", Field::flag_nullable) { }

void BasicStatfsSystemCallTraceReplayModule::print_specific_fields() {
  LOG_INFO("file system type(" << statfs_result_type_.val() << "), " \
  << "block size(" << statfs_result_bsize_.val() << "), " \
  << "total blocks(" << statfs_result_blocks_.val() << "), " \
  << "free blocks(" << statfs_result_bfree_.val() << "), " \
  << "available blocks(" << statfs_result_bavail_.val() << "), " \
  << "total file nodes(" << statfs_result_files_.val() << "), " \
  << "free file nodes(" << statfs_result_ffree_.val() << "), " \
  << "Maximum namelength(" << statfs_result_namelen_.val() << ") " \
  << "fragment size(" << statfs_result_frsize_.val() << ") " \
  << "mount flags(" << statfs_result_flags_.val() << ")");
}

void BasicStatfsSystemCallTraceReplayModule::verifyResult(
				struct statfs replayed_statfs_buf) {

  typedef unsigned long u_long;
  u_int statfs_result_type = (u_int) statfs_result_type_.val();
  u_int statfs_result_bsize = (u_int) statfs_result_bsize_.val();
  u_long statfs_result_blocks = (u_long) statfs_result_blocks_.val();
  u_long statfs_result_bfree = (u_long) statfs_result_bfree_.val();
  u_long statfs_result_bavail = (u_long) statfs_result_bavail_.val();
  u_long statfs_result_files = (u_long) statfs_result_files_.val();
  u_long statfs_result_ffree = (u_long) statfs_result_ffree_.val();
  u_long statfs_result_namelen = (u_long) statfs_result_namelen_.val();
  u_long statfs_result_frsize = (u_long) statfs_result_frsize_.val();
  u_long statfs_result_flags = (u_long) statfs_result_flags_.val();

  // Verify statfs buffer contents in the trace file are same
  if (statfs_result_type != replayed_statfs_buf.f_type ||
      statfs_result_bsize != replayed_statfs_buf.f_bsize ||
      statfs_result_blocks != replayed_statfs_buf.f_blocks ||
      statfs_result_bfree != replayed_statfs_buf.f_bfree ||
      statfs_result_bavail != replayed_statfs_buf.f_bavail ||
      statfs_result_files != replayed_statfs_buf.f_files ||
      statfs_result_ffree != replayed_statfs_buf.f_ffree ||
      statfs_result_namelen != (u_long) replayed_statfs_buf.f_namelen ||
      statfs_result_frsize != (u_long) replayed_statfs_buf.f_frsize ||
      statfs_result_flags != (u_long) replayed_statfs_buf.f_flags) {

      // Statfs buffers aren't same
      LOG_ERR("Verification of " << sys_call_name_ \
	      << " buffer content failed.");
      if (!default_mode()) {
	LOG_WARN("time called:" << std::fixed << Tfrac_to_sec(time_called()) \
		 << "Captured " << sys_call_name_ \
		 << " content is different from replayed " \
		 << sys_call_name_ << " content");
	LOG_WARN("Captured file system type: " << statfs_result_type << ", " \
		 << "Replayed file system type: " << replayed_statfs_buf.f_type);
	LOG_WARN("Captured block size: " << statfs_result_bsize << ", " \
		 << "Replayed block size: " << replayed_statfs_buf.f_bsize);
	LOG_WARN("Captured total blocks: " << statfs_result_blocks << ", " \
		 << "Replayed total blocks: " << replayed_statfs_buf.f_blocks);
	LOG_WARN("Captured free blocks: " << statfs_result_bfree << ", " \
		 << "Replayed free blocks: " << replayed_statfs_buf.f_bfree);
	LOG_WARN("Captured available blocks: " << statfs_result_bavail << ", " \
		 << "Replayed available blocks: " \
		 << replayed_statfs_buf.f_bavail);
	LOG_WARN("Captured total file inodes: " << statfs_result_files << ", " \
		 << "Replayed total file inodes: " \
		 << replayed_statfs_buf.f_files);
	LOG_WARN("Captured free file nodes: " << statfs_result_ffree << ", " \
		 << "Replayed free file nodes: " << replayed_statfs_buf.f_ffree);
	LOG_WARN("Captured maximum namelength: " << statfs_result_namelen << ", " \
		 << "Replayed maximum namelength: " \
		 << replayed_statfs_buf.f_namelen);
	LOG_WARN("Captured fragment size: " << statfs_result_frsize << ", " \
		 << "Replayed fragment size: " << replayed_statfs_buf.f_frsize);
	LOG_WARN("Captured mount flags: " << statfs_result_flags << ", " \
		 << "Replayed mount flags: " << replayed_statfs_buf.f_flags);

	if (abort_mode()) {
	  abort();
	}
      }
    } else {
      if (verbose_mode()) {
	LOG_INFO("Verification of " << sys_call_name_ \
		 << " buffer succeeded.");
      }
  }
}

StatfsSystemCallTraceReplayModule::
StatfsSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  bool verify_flag,
				  int warn_level_flag):
  BasicStatfsSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
					 warn_level_flag),
  given_pathname_(series, "given_pathname") {
  sys_call_name_ = "statfs";
}

void StatfsSystemCallTraceReplayModule::print_specific_fields() {
  LOG_INFO("pathname(" << given_pathname_.val() << "),");
  BasicStatfsSystemCallTraceReplayModule::print_specific_fields();
}

void StatfsSystemCallTraceReplayModule::processRow() {
  struct statfs statfs_buf;
  char *pathname = (char *) given_pathname_.val();

  // replay the statfs system call
  replayed_ret_val_ = statfs(pathname, &statfs_buf);

  if (verify_ == true) {
    BasicStatfsSystemCallTraceReplayModule::verifyResult(statfs_buf);
  }
}

FStatfsSystemCallTraceReplayModule::
FStatfsSystemCallTraceReplayModule(DataSeriesModule &source,
				   bool verbose_flag,
				   bool verify_flag,
				   int warn_level_flag):
  BasicStatfsSystemCallTraceReplayModule(source, verbose_flag, verify_flag,
					 warn_level_flag),
  descriptor_(series, "descriptor") {
  sys_call_name_ = "fstatfs";
}

void FStatfsSystemCallTraceReplayModule::print_specific_fields() {
  LOG_INFO("descriptor(" << descriptor_.val() << "),");
  BasicStatfsSystemCallTraceReplayModule::print_specific_fields();
}

void FStatfsSystemCallTraceReplayModule::processRow() {
  struct statfs statfs_buf;
  int fd = SystemCallTraceReplayModule::fd_map_[descriptor_.val()];

  // replay the fstatfs system call
  replayed_ret_val_ = fstatfs(fd, &statfs_buf);

  if (verify_ == true) {
    BasicStatfsSystemCallTraceReplayModule::verifyResult(statfs_buf);
  }
}
