/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file implements all the functions in the
 * RenameSystemCallTraceReplayModule header file
 *
 * Read RenameSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "RenameSystemCallTraceReplayModule.hpp"

RenameSystemCallTraceReplayModule::RenameSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      given_oldname_(series, "given_oldname", Field::flag_nullable),
      given_newname_(series, "given_newname", Field::flag_nullable) {
  sys_call_name_ = "rename";
}

void RenameSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("old name(", old_pathname, "), ", "new name(",
                            new_pathname, ")");
}

void RenameSystemCallTraceReplayModule::processRow() {
  // Replay the rename system call
  replayed_ret_val_ = rename(old_pathname, new_pathname);
  delete[] old_pathname;
  delete[] new_pathname;
}

void RenameSystemCallTraceReplayModule::prepareRow() {
  auto old_pathbuf = reinterpret_cast<const char *>(given_oldname_.val());
  old_pathname = new char[std::strlen(old_pathbuf) + 1];
  std::strcpy(old_pathname, old_pathbuf);
  auto new_pathbuf = reinterpret_cast<const char *>(given_newname_.val());
  new_pathname = new char[std::strlen(new_pathbuf) + 1];
  std::strcpy(new_pathname, new_pathbuf);
  SystemCallTraceReplayModule::prepareRow();
}
