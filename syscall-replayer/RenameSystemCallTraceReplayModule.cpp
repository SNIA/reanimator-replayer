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
 * RenameSystemCallTraceReplayModule header file
 *
 * Read RenameSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "RenameSystemCallTraceReplayModule.hpp"

RenameSystemCallTraceReplayModule::
RenameSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_oldname_(series, "given_oldname"),
  given_newname_(series, "given_newname") {
  sys_call_name_ = "rename";
}

void RenameSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "old name(" << given_oldname_.val() << "), ";
  std::cout << "new name(" << given_newname_.val() << ")";
}

void RenameSystemCallTraceReplayModule::processRow() {
  const char *old_name = (const char *)given_oldname_.val();
  const char *new_name = (const char *)given_newname_.val();

  // Replay the rename system call
  replayed_ret_val_ = rename(old_name, new_name);
}
