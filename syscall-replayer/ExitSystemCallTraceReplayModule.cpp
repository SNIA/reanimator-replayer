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
 * ExitSystemCallTraceReplayModule header file.
 *
 * Read ExitSystemCallTraceReplayModule.hpp for more information
 * about this class.
 */

#include "ExitSystemCallTraceReplayModule.hpp"
#include <climits>

ExitSystemCallTraceReplayModule::ExitSystemCallTraceReplayModule(
    DataSeriesModule &source, bool verbose_flag, int warn_level_flag)
    : SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
      exit_status_(series, "exit_status"),
      generated_(series, "generated") {
  sys_call_name_ = "exit";
}

void ExitSystemCallTraceReplayModule::print_specific_fields() {
  syscall_logger_->log_info("exit_status(", exitStat, "), ", "generated(",
                            generated, ")");
}

/*
 * NOTE: On replaying exit system call, our replayer will terminate.
 * Hence we do not replay exit system call, but we update replayer resources
 */
void ExitSystemCallTraceReplayModule::processRow() {
  // Remove umask table
  SystemCallTraceReplayModule::replayer_resources_manager_.remove_umask(
      executingPidVal);
  // Remove fd table
  auto fds_to_close =
      replayer_resources_manager_.remove_fd_table(executingPidVal);
  for (auto fd : fds_to_close) {
    close(fd);
  }
}

void ExitSystemCallTraceReplayModule::prepareRow() {
  exitStat = exit_status_.val();
  generated = generated_.val();
  SystemCallTraceReplayModule::prepareRow();
  timeReturnedVal = ULLONG_MAX;
}
