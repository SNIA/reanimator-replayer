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

#define F_SET_RW_HINT 1030

void DataSeriesOutputModule::makeFcntlArgsMap(void **args_map, long *args,
                                              void **v_args) {
  // Set all non-nullable boolean fields to false
  initArgsMap(args_map, "fcntl");

  // Save the descriptor and command value to the map
  args_map[SYSCALL_FIELD_DESCRIPTOR] = &args[0];
  args_map[SYSCALL_FIELD_COMMAND_VALUE] = &args[1];

  int command = args[1];
  /*
   * Check the command argument passed to fcntl and set the corresponding
   * fields in the map
   */
  switch (command) {
    // File descriptor dup command
    case F_DUPFD:
      args_map[SYSCALL_FIELD_COMMAND_DUP] = &true_;
      args_map[SYSCALL_FIELD_ARGUMENT_VALUE] = &args[2];
      break;

    // Get file descriptor flags command
    case F_GETFD:
      args_map[SYSCALL_FIELD_COMMAND_GET_DESCRIPTOR_FLAGS] = &true_;
      break;

    // Set file descriptor flags command
    case F_SETFD: {
      args_map[SYSCALL_FIELD_COMMAND_SET_DESCRIPTOR_FLAGS] = &true_;
      args_map[SYSCALL_FIELD_ARGUMENT_VALUE] = &args[2];
      u_int fd_flag = (u_int)args[2];
      process_Flag_and_Mode_Args(
          args_map, fd_flag, FD_CLOEXEC,
          SYSCALL_FIELD_ARGUMENT_DESCRIPTOR_FLAG_EXEC_CLOSE);
      if (fd_flag != 0) {
        std::cerr << "Fcntl: SETFD: These flags are not processed/unknown->0x"
                  << std::hex << fd_flag << std::dec << std::endl;
      }
      break;
    }

    case F_SET_RW_HINT: {
      args_map[SYSCALL_FIELD_ARGUMENT_VALUE] = &args[2];
      break;
    }

    // Get file status flags command
    case F_GETFL:
      args_map[SYSCALL_FIELD_COMMAND_GET_STATUS_FLAGS] = &true_;
      break;

    // Set file status flags command
    case F_SETFL: {
      args_map[SYSCALL_FIELD_COMMAND_SET_STATUS_FLAGS] = &true_;
      args_map[SYSCALL_FIELD_ARGUMENT_VALUE] = &args[2];
      u_int status_flag = processFcntlStatusFlags(args_map, args[2]);
      if (status_flag != 0) {
        std::cerr << "Fcntl: SETFL: These flags are not processed/unknown->0x"
                  << std::hex << status_flag << std::dec << std::endl;
      }
      break;
    }
    // Set lock command
    case F_SETLK:
      args_map[SYSCALL_FIELD_COMMAND_SET_LOCK] = &true_;
      processFcntlFlock(args_map, (struct flock *)v_args[0]);
      break;

    // Set lock wait command
    case F_SETLKW:
      args_map[SYSCALL_FIELD_COMMAND_SET_LOCK_WAIT] = &true_;
      processFcntlFlock(args_map, (struct flock *)v_args[0]);
      break;

    // Get lock command
    case F_GETLK:
      args_map[SYSCALL_FIELD_COMMAND_GET_LOCK] = &true_;
      processFcntlFlock(args_map, (struct flock *)v_args[0]);
      break;

    // Get process id command
    case F_GETOWN:
      args_map[SYSCALL_FIELD_COMMAND_GET_PROCESS_ID] = &true_;
      break;

    // Set process id command
    case F_SETOWN:
      args_map[SYSCALL_FIELD_COMMAND_SET_PROCESS_ID] = &true_;
      args_map[SYSCALL_FIELD_ARGUMENT_VALUE] = &args[2];
      break;

    // Get signal command
    case F_GETSIG:
      args_map[SYSCALL_FIELD_COMMAND_GET_SIGNAL] = &true_;
      break;

    // Set signal command
    case F_SETSIG:
      args_map[SYSCALL_FIELD_COMMAND_SET_SIGNAL] = &true_;
      args_map[SYSCALL_FIELD_ARGUMENT_VALUE] = &args[2];
      break;

    // Get lease command
    case F_GETLEASE: {
      args_map[SYSCALL_FIELD_COMMAND_GET_LEASE] = &true_;
      int return_value = *(int *)args_map[SYSCALL_FIELD_RETURN_VALUE];
      processFcntlLease(args_map, return_value);
      break;
    }
    // Set lease command
    case F_SETLEASE:
      args_map[SYSCALL_FIELD_COMMAND_SET_LEASE] = &true_;
      args_map[SYSCALL_FIELD_ARGUMENT_VALUE] = &args[2];
      processFcntlLease(args_map, args[2]);
      break;

    // Notify command
    case F_NOTIFY: {
      args_map[SYSCALL_FIELD_COMMAND_NOTIFY] = &true_;
      args_map[SYSCALL_FIELD_ARGUMENT_VALUE] = &args[2];
      u_int notify_value = processFcntlNotify(args_map, args);
      if (notify_value != 0) {
        std::cerr << "Fcntl: F_NOTIFY: These flags are not processed/unknown->"
                  << std::hex << notify_value << std::dec << std::endl;
      }
      break;
    }
    /*
     * If the command value doesn't match a known command, print
     * a warning message
     */
    default:
      std::cerr << "Fcntl: Command is unknown->" << command << std::endl;
      args_map[SYSCALL_FIELD_ARGUMENT_VALUE] = &args[2];
  }
}

/*
 * This function unwraps the flag value passed as an argument to the
 * fcntl system call with the F_SETFL command and sets the corresponding
 * flag values as True.
 *
 * @param args_map: stores mapping of <field, value> pairs.
 *
 * @param status_flag: represents the flag value passed as an argument
 *                   to the fcntl system call.
 */
u_int DataSeriesOutputModule::processFcntlStatusFlags(void **args_map,
                                                      u_int status_flag) {
  /*
   * Process each individual flag bit that has been set
   * in the argument status_flag.
   */
  // set read only flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_RDONLY,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_READ_ONLY);
  // set write only flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_WRONLY,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_WRITE_ONLY);
  // set read and write flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_RDWR,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_READ_AND_WRITE);
  // set append flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_APPEND,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_APPEND);
  // set async flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_ASYNC,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_ASYNC);
  // set create flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_CREAT,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_CREATE);
  // set direct flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_DIRECT,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_DIRECT);
  // set directory flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_DIRECTORY,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_DIRECTORY);
  // set exclusive flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_EXCL,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_EXCLUSIVE);
  // set largefile flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_LARGEFILE,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_LARGEFILE);
  // set last access time flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_NOATIME,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_ACCESS_TIME);
  // set controlling terminal flag
  process_Flag_and_Mode_Args(
      args_map, status_flag, O_NOCTTY,
      SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_CONTROLLING_TERMINAL);
  // set no_follow flag (in case of symbolic link)
  process_Flag_and_Mode_Args(args_map, status_flag, O_NOFOLLOW,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_FOLLOW);
  // set non blocking mode flag
  process_Flag_and_Mode_Args(
      args_map, status_flag, O_NONBLOCK,
      SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_BLOCKING_MODE);
  // set no delay flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_NDELAY,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_DELAY);
  // set synchronized IO flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_SYNC,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_SYNCHRONOUS);
  // set truncate mode flag
  process_Flag_and_Mode_Args(args_map, status_flag, O_TRUNC,
                             SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_TRUNCATE);

  /*
   * Return remaining unprocessed flags so that caller can
   * warn of unknown flags if the status_flag value is not set
   * as zero.
   */
  return status_flag;
}

/*
 * This function saves the values in the flock structure passed to
 * Fcntl with command F_SETLK, F_SETLKW, or F_GETLK into the map
 */
void DataSeriesOutputModule::processFcntlFlock(void **args_map,
                                               struct flock *lock) {
  if (lock != NULL) {
    // Save the values in the flock structure to the map
    processFcntlFlockType(args_map, lock);
    processFcntlFlockWhence(args_map, lock);
    args_map[SYSCALL_FIELD_LOCK_START] = &lock->l_start;
    args_map[SYSCALL_FIELD_LOCK_LENGTH] = &lock->l_len;
    args_map[SYSCALL_FIELD_LOCK_PID] = &lock->l_pid;
  } else {
    /*
     * If the flock passed to Fcntl was NULL, then print a warning message.
     * The int32 fields lock_type, lock_whence, lock_start, lock_length,
     * and lock_pid will be set to 0 by default.
     */
    std::cerr << "Flock: Struct flock is set as NULL!!" << std::endl;
  }
}

/*
 * This function processes the l_type member of an flock structure
 * and sets the corresponding field in the map
 */
void DataSeriesOutputModule::processFcntlFlockType(void **args_map,
                                                   struct flock *lock) {
  // Save the lock type value into the map
  args_map[SYSCALL_FIELD_LOCK_TYPE] = &lock->l_type;
  u_int type = lock->l_type;

  /*
   * If the type value matches one of the possible types, set the
   * corresponding field in the map to True
   */
  switch (type) {
    // set read lock field
    case F_RDLCK:
      args_map[SYSCALL_FIELD_LOCK_TYPE_READ] = &true_;
      break;
    // set write lock field
    case F_WRLCK:
      args_map[SYSCALL_FIELD_LOCK_TYPE_WRITE] = &true_;
      break;
    // set unlocked field
    case F_UNLCK:
      args_map[SYSCALL_FIELD_LOCK_TYPE_UNLOCKED] = &true_;
      break;
    // If the type value isn't a known type, print a warning message
    default:
      std::cerr << "Fcntl: Lock type is unknown->" << lock << std::endl;
  }
}

/*
 * This function processes the l_whence member of an flock structure
 * and sets the corresponding field in the map
 */
void DataSeriesOutputModule::processFcntlFlockWhence(void **args_map,
                                                     struct flock *lock) {
  // Save the lock whence value into the map
  args_map[SYSCALL_FIELD_LOCK_WHENCE] = &lock->l_whence;
  u_int whence = lock->l_whence;

  /*
   * If the whence value matches one of the possible values, set the
   * corresponding field in the map to True
   */
  switch (whence) {
    // set SEEK_SET whence field
    case SEEK_SET:
      args_map[SYSCALL_FIELD_LOCK_WHENCE_START] = &true_;
      break;
    // set SEEK_CUR whence field
    case SEEK_CUR:
      args_map[SYSCALL_FIELD_LOCK_WHENCE_CURRENT] = &true_;
      break;
    // set SEEK_END whence field
    case SEEK_END:
      args_map[SYSCALL_FIELD_LOCK_WHENCE_END] = &true_;
      break;
    // If the whence value isn't a known whence value, print a warning message
    default:
      std::cerr << "Fcntl: Lock whence is unknown->" << whence << std::endl;
  }
}

/*
 * This function processes the lease value passed to an Fcntl system call with
 * an F_SETLEASE command or returned from an F_GETLEASE command
 */
void DataSeriesOutputModule::processFcntlLease(void **args_map, int lease) {
  /*
   * If the lease argument matches one of the possible values, set the
   * corresponding field in the map to True
   */
  switch (lease) {
    // set read lock lease field
    case F_RDLCK:
      args_map[SYSCALL_FIELD_ARGUMENT_LEASE_READ] = &true_;
      break;
    // set write lock lease field
    case F_WRLCK:
      args_map[SYSCALL_FIELD_ARGUMENT_LEASE_WRITE] = &true_;
      break;
    // set unlocked lease field
    case F_UNLCK:
      args_map[SYSCALL_FIELD_ARGUMENT_LEASE_REMOVE] = &true_;
      break;
    // If the lease argument isn't a known lease, print a warning message
    default:
      std::cerr << "Fcntl: Lease argument is unknown->" << lease << std::endl;
  }
}

/*
 * This function processes the notify value passed to an Fcntl system call
 * with an F_NOTIFY command.  It returns any unprocessed notify_value bits.
 */
u_int DataSeriesOutputModule::processFcntlNotify(void **args_map, long *args) {
  u_int notify_value = args[2];

  // set access argument bit
  process_Flag_and_Mode_Args(args_map, notify_value, DN_ACCESS,
                             SYSCALL_FIELD_ARGUMENT_NOTIFY_ACCESS);
  // set access argument bit
  process_Flag_and_Mode_Args(args_map, notify_value, DN_MODIFY,
                             SYSCALL_FIELD_ARGUMENT_NOTIFY_MODIFY);
  // set access argument bit
  process_Flag_and_Mode_Args(args_map, notify_value, DN_CREATE,
                             SYSCALL_FIELD_ARGUMENT_NOTIFY_CREATE);
  // set access argument bit
  process_Flag_and_Mode_Args(args_map, notify_value, DN_DELETE,
                             SYSCALL_FIELD_ARGUMENT_NOTIFY_DELETE);
  // set access argument bit
  process_Flag_and_Mode_Args(args_map, notify_value, DN_RENAME,
                             SYSCALL_FIELD_ARGUMENT_NOTIFY_RENAME);
  // set access argument bit
  process_Flag_and_Mode_Args(args_map, notify_value, DN_ATTRIB,
                             SYSCALL_FIELD_ARGUMENT_NOTIFY_ATTRIBUTE);

  /*
   * Return remaining notify flags so that caller can
   * warn of unknown flags if the notify_value is not set
   * as zero.
   */
  return notify_value;
}
