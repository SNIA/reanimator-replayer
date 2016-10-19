/*
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Nina Brown
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Geoff Kuenning
 * Copyright (c) 2015-2016 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions for implementing file
 * descriptor manager.
 *
 * FileDescriptorManager is a class that has members and functions of
 * managing file descriptors during replaying.
 *
 * USAGE
 * A main program could initialize this class and call desired
 * function to add/delete/update file descriptor table.
 */

#ifndef FILE_DESCRIPTOR_MANAGER_HPP
#define FILE_DESCRIPTOR_MANAGER_HPP

#include "SystemCallTraceReplayLogger.hpp"

#include <utility>
#include <map>
#include <unordered_set> 
#include <string>
#include <vector>
#include <iterator>

// <replayed fd, flags>
typedef std::pair<int, int> FileDescriptor;
// <traced fd, replayed FileDescriptor>
typedef std::map<int, FileDescriptor> FileDescriptorTable;
// <pid, FileDescriptorTable>
typedef std::map<int, FileDescriptorTable> PerPidFileDescriptorTableMap;
// <traced fd, reference count>
typedef std::map<int, int> FileDescriptorReferenceCount;
class FileDescriptorManager {
private:
  PerPidFileDescriptorTableMap fd_table_map_;
  FileDescriptorReferenceCount fd_rc_;

public:
  // Constructor
  FileDescriptorManager();

  /*
   * This function needs to be called first before
   * using any method in this class for the following reason.
   * When sys call replayer starts, there is no fd table
   * for any process. We need to create the fd table for
   * the first process before replaying.
   */
  void initialize(pid_t pid, std::map<int, int>& fd_map);

  /*
   * This function will add a file descriptor mapping.
   * It will increase the reference count and create a file
   * descriptor if it hasn't been created yet.
   */
  void add_fd(pid_t pid, int traced_fd, int replayed_fd, int flags);

  /*
   * This function will remove a file descriptor mapping.
   * It will decrease the reference count and remove the file descriptor from
   * the table.
   * Return a pair where the first element indicates whether the caller needs to
   * close the file or not and the second element is replayed fd if no error.
   */
  std::pair<bool, int> remove_fd(pid_t pid, int traced_fd);

  /*
   * This function will return replayed file descriptor that corrsponds
   * to given traced file descriptor.
   */
  int get_fd(pid_t pid, int traced_fd);

  /*
   * This function will add flag to a file descriptor.
   */
  void add_flags(pid_t pid, int traced_fd, int flag);

  /*
   * This function will remove flags from a file descriptor.
   */
  void remove_flags(pid_t pid, int traced_fd, int flag);

  /*
   * This function will clone file descriptor table for a process.
   * Increment the reference count for all traced fd in the parent process.
   * Return -1 to indicate a failure, otherwise, return 0.
   */
  int clone_fd_table(pid_t ppid, pid_t pid);

  /*
   * This function will remove a file descriptor table.
   * It will decrease the reference count for each fd in the file
   * descriptor table.
   * Return a list of fds that need to be closed
   */
  std::vector<std::pair<bool, int> > remove_fd_table(pid_t pid);
};

#endif /* FILE_DESCRIPTOR_MANAGER_HPP */
