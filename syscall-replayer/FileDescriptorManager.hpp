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

#include <utility>
#include <map>
#include <vector>
#include <string>
#include <iterator>

typedef std::pair<int, std::vector<std::string> > FileDescriptor;
typedef std::map<int, FileDescriptor> FileDescriptorTable;
typedef std::map<int, FileDescriptorTable> FileDescriptorTableMap;
typedef std::map<int, int> FileDescriptorReferenceCount;
class FileDescriptorManager {
private:
  // Access System Call Trace Fields in Dataseries file
  FileDescriptorTableMap fd_table_map;
  FileDescriptorReferenceCount fd_rc;

public:
  // Constructor
  FileDescriptorManager();

  /*
   * This function will add a file descriptor mapping.
   * It will increase the reference count and create a file
   * descriptor if it hasn't been created yet.
   */
  void add_fd(int pid, int traced_fd, int replayed_fd, std::vector<std::string> flags);

  /*
   * This function will add flag to a file descriptor.
   */
  void add_flag(int pid, int traced_fd, std::string flag);

  /*
   * This function will add flags to a file descriptor.
   */
  void add_flags(int pid, int traced_fd, std::vector<std::string> flags);

  /*
   * This function will clone file descriptor table for a process
   */
  void clone_fd_table(int ppid, int pid);

  /*
   * This function will update a file descriptor mapping
   * based on the given arguments.
   */
  void update(int pid, int traced_fd, int replayed_fd, std::vector<std::string> flags);

  /*
   * This function will add a file descriptor mapping.
   * It will increase the reference count and create a file
   * descriptor if it hasn't been created yet.
   * Return -1 if reference count reaches 0, otherwise return false.
   */
  int remove_fd(int pid, int traced_fd);

  /*
   * This function will remove a file descriptor table.
   * It will decrease the reference count for each fd in the file
   * descriptor table.
   * Return a list of fds that need to be closed
   */
  std::vector<int> remove_fd_table(int pid);

  /*
   * This function will return replayed file descriptor that corrsponds
   * to given traced file descriptor.
   */
  int get_fd(int pid, int traced_fd);
};

#endif /* FILE_DESCRIPTOR_MANAGER_HPP */
