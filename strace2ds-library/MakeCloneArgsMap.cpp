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
 * This file implements all the functions in the DataSeriesOutputModule.hpp
 * header file.
 *
 * Read the DataSeriesOutputModule.hpp file for more information about this
 * class.
 */

#include "DataSeriesOutputModule.hpp"

void DataSeriesOutputModule::makeCloneArgsMap(void **args_map, long *args,
                                              void **v_args) {
  initArgsMap(args_map, "clone");

  args_map[SYSCALL_FIELD_FLAG_VALUE] = &args[0];
  u_int flag = processCloneFlags(args_map, args[0]);
  flag = processCloneSignals(args_map, flag);
  if (flag != 0) {
    std::cerr << "Clone: These flags are not processed/unknown->0x";
    std::cerr << std::hex << flag << std::dec << std::endl;
  }

  args_map[SYSCALL_FIELD_CHILD_STACK_ADDRESS] = &args[1];

  if (v_args[0] != NULL) {
    args_map[SYSCALL_FIELD_PARENT_THREAD_ID] = &v_args[0];
  } else if (args[0] & CLONE_PARENT_SETTID) {
    std::cerr << "Clone: Parent thread ID is set as NULL!!" << std::endl;
  }

  if (v_args[1] != NULL) {
    args_map[SYSCALL_FIELD_CHILD_THREAD_ID] = &v_args[1];
  } else if ((args[0] & CLONE_CHILD_SETTID) ||
             (args[0] & CLONE_CHILD_CLEARTID)) {
    std::cerr << "Clone: Child thread ID is set as NULL!!" << std::endl;
  }

  if (clone_ctid_index_ == 3)
    args_map[SYSCALL_FIELD_NEW_TLS] = &args[4];
  else
    args_map[SYSCALL_FIELD_NEW_TLS] = &args[3];
}

u_int DataSeriesOutputModule::processCloneFlags(void **args_map, u_int flag) {
  /*
   * Process each individual clone flag bit that has been set.
   */
  // set child_cleartid flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_CHILD_CLEARTID,
                             SYSCALL_FIELD_FLAG_CHILD_CLEARTID);

  // set child_settid flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_CHILD_SETTID,
                             SYSCALL_FIELD_FLAG_CHILD_SETTID);

  // set files flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_FILES,
                             SYSCALL_FIELD_FLAG_FILES);

  // set filesystem flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_FS,
                             SYSCALL_FIELD_FLAG_FILESYSTEM);

  // set I/O flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_IO, SYSCALL_FIELD_FLAG_IO);

  // set newipc flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWIPC,
                             SYSCALL_FIELD_FLAG_NEWIPC);

  // set newnet flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWNET,
                             SYSCALL_FIELD_FLAG_NEWNET);

  // set newns flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWNS,
                             SYSCALL_FIELD_FLAG_NEWNS);

  // set newpid flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWPID,
                             SYSCALL_FIELD_FLAG_NEWPID);

  // set newuser flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWUSER,
                             SYSCALL_FIELD_FLAG_NEWUSER);

  // set newuts flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_NEWUTS,
                             SYSCALL_FIELD_FLAG_NEWUTS);

  // set parent flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_PARENT,
                             SYSCALL_FIELD_FLAG_PARENT);

  // set parent_settid flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_PARENT_SETTID,
                             SYSCALL_FIELD_FLAG_PARENT_SETTID);

  // set ptrace flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_PTRACE,
                             SYSCALL_FIELD_FLAG_PTRACE);

  // set settls flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_SETTLS,
                             SYSCALL_FIELD_FLAG_SETTLS);

  // set sighand flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_SIGHAND,
                             SYSCALL_FIELD_FLAG_SIGHAND);

  // set sysvsem flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_SYSVSEM,
                             SYSCALL_FIELD_FLAG_SYSVSEM);

  // set thread flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_THREAD,
                             SYSCALL_FIELD_FLAG_THREAD);

  // set untraced flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_UNTRACED,
                             SYSCALL_FIELD_FLAG_UNTRACED);

  // set vfork flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_VFORK,
                             SYSCALL_FIELD_FLAG_VFORK);

  // set vm flag
  process_Flag_and_Mode_Args(args_map, flag, CLONE_VM, SYSCALL_FIELD_FLAG_VM);

  return flag;
}

u_int DataSeriesOutputModule::processCloneSignals(void **args_map, u_int flag) {
  /*
   * Process each individual clone signal bit that has been set in the flags
   * passed to clone
   */
  // set signal_hangup field
  process_Flag_and_Mode_Args(args_map, flag, SIGHUP,
                             SYSCALL_FIELD_SIGNAL_HANGUP);
  // set signal_terminal_interrupt field
  process_Flag_and_Mode_Args(args_map, flag, SIGINT,
                             SYSCALL_FIELD_SIGNAL_TERMINAL_INTERRUPT);

  // set signal_terminal_quit field
  process_Flag_and_Mode_Args(args_map, flag, SIGQUIT,
                             SYSCALL_FIELD_SIGNAL_TERMINAL_QUIT);

  // set signal_illegal field
  process_Flag_and_Mode_Args(args_map, flag, SIGILL,
                             SYSCALL_FIELD_SIGNAL_ILLEGAL);

  // set signal_trace_trap field
  process_Flag_and_Mode_Args(args_map, flag, SIGTRAP,
                             SYSCALL_FIELD_SIGNAL_TRACE_TRAP);

  // set signal_abort field
  process_Flag_and_Mode_Args(args_map, flag, SIGABRT,
                             SYSCALL_FIELD_SIGNAL_ABORT);

  // set signal_iot_trap field
  process_Flag_and_Mode_Args(args_map, flag, SIGIOT,
                             SYSCALL_FIELD_SIGNAL_IOT_TRAP);

  // set signal_bus field
  process_Flag_and_Mode_Args(args_map, flag, SIGBUS, SYSCALL_FIELD_SIGNAL_BUS);

  // set signal_floating_point_exception field
  process_Flag_and_Mode_Args(args_map, flag, SIGFPE,
                             SYSCALL_FIELD_SIGNAL_FLOATING_POINT_EXCEPTION);

  // set signal_kill field
  process_Flag_and_Mode_Args(args_map, flag, SIGKILL,
                             SYSCALL_FIELD_SIGNAL_KILL);

  // set signal_user_defined_1 field
  process_Flag_and_Mode_Args(args_map, flag, SIGUSR1,
                             SYSCALL_FIELD_SIGNAL_USER_DEFINED_1);

  // set signal_segv field
  process_Flag_and_Mode_Args(args_map, flag, SIGSEGV,
                             SYSCALL_FIELD_SIGNAL_SEGV);

  // set signal_user_defined_2 field
  process_Flag_and_Mode_Args(args_map, flag, SIGUSR2,
                             SYSCALL_FIELD_SIGNAL_USER_DEFINED_2);

  // set signal_pipe field
  process_Flag_and_Mode_Args(args_map, flag, SIGPIPE,
                             SYSCALL_FIELD_SIGNAL_PIPE);

  // set signal_alarm field
  process_Flag_and_Mode_Args(args_map, flag, SIGALRM,
                             SYSCALL_FIELD_SIGNAL_ALARM);

  // set signal_termination field
  process_Flag_and_Mode_Args(args_map, flag, SIGTERM,
                             SYSCALL_FIELD_SIGNAL_TERMINATION);

  // set signal_stack_fault field
  process_Flag_and_Mode_Args(args_map, flag, SIGSTKFLT,
                             SYSCALL_FIELD_SIGNAL_STACK_FAULT);

  // set signal_child field
  process_Flag_and_Mode_Args(args_map, flag, SIGCHLD,
                             SYSCALL_FIELD_SIGNAL_CHILD);

  // set signal_continue field
  process_Flag_and_Mode_Args(args_map, flag, SIGCONT,
                             SYSCALL_FIELD_SIGNAL_CONTINUE);

  // set signal_stop field
  process_Flag_and_Mode_Args(args_map, flag, SIGSTOP,
                             SYSCALL_FIELD_SIGNAL_STOP);

  // set signal_terminal_stop field
  process_Flag_and_Mode_Args(args_map, flag, SIGTSTP,
                             SYSCALL_FIELD_SIGNAL_TERMINAL_STOP);

  // set signal_tty_read field
  process_Flag_and_Mode_Args(args_map, flag, SIGTTIN,
                             SYSCALL_FIELD_SIGNAL_TTY_READ);

  // set signal_tty_write field
  process_Flag_and_Mode_Args(args_map, flag, SIGTTOU,
                             SYSCALL_FIELD_SIGNAL_TTY_WRITE);

  // set signal_urgent field
  process_Flag_and_Mode_Args(args_map, flag, SIGURG,
                             SYSCALL_FIELD_SIGNAL_URGENT);

  // set signal_cpu_exceeded field
  process_Flag_and_Mode_Args(args_map, flag, SIGXCPU,
                             SYSCALL_FIELD_SIGNAL_CPU_EXCEEDED);

  // set signal_file_size_exceeded field
  process_Flag_and_Mode_Args(args_map, flag, SIGXFSZ,
                             SYSCALL_FIELD_SIGNAL_FILE_SIZE_EXCEEDED);

  // set signal_virtual_alarm field
  process_Flag_and_Mode_Args(args_map, flag, SIGVTALRM,
                             SYSCALL_FIELD_SIGNAL_VIRTUAL_ALARM);

  // set signal_prof_alarm field
  process_Flag_and_Mode_Args(args_map, flag, SIGPROF,
                             SYSCALL_FIELD_SIGNAL_PROF_ALARM);

  // set signal_window_size_change field
  process_Flag_and_Mode_Args(args_map, flag, SIGWINCH,
                             SYSCALL_FIELD_SIGNAL_WINDOW_SIZE_CHANGE);

  // set signal_io field
  process_Flag_and_Mode_Args(args_map, flag, SIGIO, SYSCALL_FIELD_SIGNAL_IO);

  // set signal_power field
  process_Flag_and_Mode_Args(args_map, flag, SIGPWR,
                             SYSCALL_FIELD_SIGNAL_POWER);

  return flag;
}
