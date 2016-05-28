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
 * This file implements all the functions in the StatSystemCallTraceReplayModule 
 * header file 
 *
 * Read StatSystemCallTraceReplayModule.hpp for more information about this class.
 */

#include "StatSystemCallTraceReplayModule.hpp"

StatSystemCallTraceReplayModule::StatSystemCallTraceReplayModule(DataSeriesModule &source,
								 bool verbose_flag,
                                                                 bool verify_flag,
								 int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  verify_(verify_flag),
  given_pathname_(series, "given_pathname"),
  stat_result_dev_(series, "stat_result_dev", Field::flag_nullable),
  stat_result_ino_(series, "stat_result_ino", Field::flag_nullable),
  stat_result_mode_(series, "stat_result_mode", Field::flag_nullable),
  stat_result_nlink_(series, "stat_result_nlink", Field::flag_nullable),
  stat_result_uid_(series, "stat_result_uid", Field::flag_nullable),
  stat_result_gid_(series, "stat_result_gid", Field::flag_nullable),
  stat_result_blksize_(series, "stat_result_blksize", Field::flag_nullable),
  stat_result_blocks_(series, "stat_result_blocks", Field::flag_nullable),
  stat_result_size_(series, "stat_result_size", Field::flag_nullable), 
  stat_result_atime_(series, "stat_result_atime", Field::flag_nullable),
  stat_result_mtime_(series, "stat_result_mtime", Field::flag_nullable),
  stat_result_ctime_(series, "stat_result_ctime", Field::flag_nullable) { 
  sys_call_name_ = "stat";
}

void StatSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Stat System Call Replayer starts to replay...-----" << std::endl;
}

void StatSystemCallTraceReplayModule::print_mode_value(unsigned int st_mode) {
  int printable_mode = 0;
  mode_t mode = (mode_t)st_mode;
  if(mode & S_ISUID)
    printable_mode |= 0x4000;
  if(mode & S_ISGID)
    printable_mode |= 0x2000;
  if(mode & S_ISVTX)
    printable_mode |= 0x1000;
  if(mode & S_IRUSR)
    printable_mode |= 0x400;
  if(mode & S_IWUSR)
    printable_mode |= 0x200;
  if(mode & S_IXUSR)
    printable_mode |= 0x100;
  if(mode & S_IRGRP)
    printable_mode |= 0x040;
  if(mode & S_IWGRP)
    printable_mode |= 0x020;
  if(mode & S_IXGRP)
    printable_mode |= 0x010;
  if(mode & S_IROTH)
    printable_mode |= 0x0004;
  if(mode & S_IWOTH)
    printable_mode |= 0x002;
  if(mode & S_IXOTH)
    printable_mode |= 0x001;

  std::cout << std::hex << printable_mode;
}

void StatSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "devie id(" << stat_result_dev_.val() << "), ";
  std::cout << "file inode number(" << stat_result_ino_.val() << "), ";
  std::cout << "file mode(";
  print_mode_value(stat_result_mode_.val());
  std::cout << "), ";
  std::cout << "file nlinks(" << stat_result_nlink_.val() << "), ";
  std::cout << "file UID(" << stat_result_uid_.val() << "), ";
  std::cout << "file GID(" << stat_result_gid_.val() << "), ";
  std::cout << "file size(" << stat_result_size_.val() << "), ";
  std::cout << "file blksize(" << stat_result_blksize_.val() << "), ";
  std::cout << "file blocks(" << stat_result_blocks_.val() << ") ";
  std::cout << "file atime(" << (double)stat_result_atime_.val() << ") ";
  std::cout << "file mtime(" << (double)stat_result_mtime_.val() << ") ";
  std::cout << "file ctime(" << (double)stat_result_ctime_.val() << ") " << std::endl;
 
}

void StatSystemCallTraceReplayModule::processRow() { 
  struct stat stat_buf;
  char *pathname = (char*)given_pathname_.val();
  unsigned int stat_result_ino = (int)stat_result_ino_.val();
  unsigned int stat_result_mode = stat_result_mode_.val();
  unsigned int stat_result_nlink = (int)stat_result_nlink_.val();
  unsigned int stat_result_uid = (int)stat_result_uid_.val();
  unsigned int stat_result_gid = (int)stat_result_gid_.val();
  long stat_result_size = stat_result_size_.val();
  int stat_result_blksize = (int)stat_result_blksize_.val();
  int stat_result_blocks = (int)stat_result_blocks_.val();

  // replay the stat system call
  replayed_ret_val_ = stat(pathname, &stat_buf);

  if (verify_ == true) {
    /* Verify stat buffer contents in the trace file are same
     * We are comparing only key fields captured in strace : st_ino,
     * st_mode, st_nlink, st_uid, st_gid, st_size, st_blksize and
     * st_blocks fileds of struct stat.
     */
    if (stat_result_ino != stat_buf.st_ino || stat_result_mode != stat_buf.st_mode ||
        stat_result_nlink != stat_buf.st_nlink || stat_result_uid != stat_buf.st_uid ||
        stat_result_gid != stat_buf.st_gid || stat_result_size != stat_buf.st_size ||
        stat_result_blksize != stat_buf.st_blksize || stat_result_blocks != stat_buf.st_blocks) {

      // Stat buffer aren't same
      std::cerr << "Verification of stat buffer content failed.\n";
      if (!default_mode()) {
	std::cout << "time called:" << std::fixed << time_called() << std::endl;
	std::cout << "Captured stat content is different from replayed stat content" << std::endl;
	std::cout << "Captured file inode: " << stat_result_ino << ", ";
	std::cout << "Replayed file inode: " << stat_buf.st_ino << std::endl;
        std::cout << "Captured file mode: ";
        print_mode_value(stat_result_mode); 
        std::cout << ", ";
	std::cout << "Replayed file mode: ";
        print_mode_value(stat_buf.st_mode); 
        std::cout << std::endl;
        std::cout << "Captured file nlink: " << stat_result_nlink << ", ";
	std::cout << "Replayed file nlink: " << stat_buf.st_nlink << std::endl;
        std::cout << "Captured file UID: " << stat_result_uid << ", ";
	std::cout << "Replayed file UID: " << stat_buf.st_uid << std::endl;
        std::cout << "Captured file GID: " << stat_result_gid << ", ";
	std::cout << "Replayed file GID: " << stat_buf.st_gid << std::endl;
        std::cout << "Captured file size: " << stat_result_size << ", ";
	std::cout << "Replayed file size: " << stat_buf.st_size << std::endl;
	std::cout << "Captured file blksize: " << stat_result_blksize << ", ";
	std::cout << "Replayed file blksize: " << stat_buf.st_blksize << std::endl;
	std::cout << "Captured file blocks: " << stat_result_blocks << ", ";
	std::cout << "Replayed file blocks: " << stat_buf.st_blocks << std::endl;
	
        if (abort_mode()) {
	  abort();
	}
      }
    } else {
      if (verbose_mode()) {
	std::cout << "Verification of stat buffer succeded.\n";
      }
    }
  }
}

void StatSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Stat System Call Replayer finished replaying...-----" << std::endl;
}

