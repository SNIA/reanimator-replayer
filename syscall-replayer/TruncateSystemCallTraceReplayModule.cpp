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
 * TruncateSystemCallTraceReplayModule header file
 *
 * Read TruncateSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "TruncateSystemCallTraceReplayModule.hpp"

#include <unistd.h>
#include <sys/types.h>

TruncateSystemCallTraceReplayModule::
TruncateSystemCallTraceReplayModule(DataSeriesModule &source,
				    bool verbose_flag,
				    int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  given_pathname_(series, "given_pathname"),
  truncate_length_(series, "truncate_length") {
  sys_call_name_ = "truncate";
}

void TruncateSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "pathname(" << given_pathname_.val() << "), ";
  std::cout << "length(" << truncate_length_.val() << ")";
}

void TruncateSystemCallTraceReplayModule::prepareForProcessing() {
  std::cout << "-----Truncate System Call Replayer starts to replay...-----"
	    << std::endl;
}

void TruncateSystemCallTraceReplayModule::processRow() {
  char *path = (char *)given_pathname_.val();
  int64_t length = truncate_length_.val();
  // Replay the truncate system call
  replayed_ret_val_ = truncate(path, length);
}

void TruncateSystemCallTraceReplayModule::completeProcessing() {
  std::cout << "-----Truncate System Call Replayer finished replaying...-----"
	    << std::endl;
}
