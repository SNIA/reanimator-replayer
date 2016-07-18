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
 * ExitSystemCallTraceReplayModule header file.
 *
 * Read ExitSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "ExitSystemCallTraceReplayModule.hpp"

ExitSystemCallTraceReplayModule::
ExitSystemCallTraceReplayModule(DataSeriesModule &source,
				bool verbose_flag,
				int warn_level_flag):
  SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
  exit_status_(series, "exit_status"),
  generated_(series, "generated") {
  sys_call_name_ = "exit";
}

void ExitSystemCallTraceReplayModule::print_specific_fields() {
  std::cout << "System call 'exit' was executed with following arguments:\n";
  std::cout << sys_call_name_ << ": " << std::endl;
  std::cout << "time_called(" << Tfrac_to_sec(time_called()) << "), ";
  std::cout << "executing pid(" << executing_pid() << "), ";
  std::cout << "unique_id(" << unique_id_.val() << "), ";
  std::cout << "exit_status(" << exit_status_.val() << "), ";
  std::cout << "generated(" << generated_.val() << ")" << std::endl;
}

void ExitSystemCallTraceReplayModule::processRow() {
  // Get exit status
  int status = exit_status_.val();

  // In verbose mode, display the exit system call fields
  if (verbose_mode()) {
    print_specific_fields();
  }

  /*
   * Replay the exit system call. On replaying our
   * replayer will terminate.
   */
  std::cout << "+++ replayer exited with exit code ("
	    << status << ") +++" << std::endl;
  exit(status);
}
