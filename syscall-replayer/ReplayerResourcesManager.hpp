/*
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Nina Brown
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2017 Geoff Kuenning
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions for implementing file
 * descriptor manager.
 *
 * ReplayerResourcesManager is a class that has members and functions of
 * managing file descriptors during replaying.
 *
 * USAGE
 * A main program could initialize this class and call desired
 * function to add/delete/update file descriptor table.
 */

#ifndef REPLAYER_RESOURCES_MANAGER_HPP
#define REPLAYER_RESOURCES_MANAGER_HPP

#include <utility>
#include <map>
#include <string>
#include <vector>
#include <unordered_set>
#include <iterator>
#include <assert.h>
#include <iostream>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include "SystemCallTraceReplayLogger.hpp"

/*
 * This number is the number of fds that we will scan (0 to MAX_FD_TO_SCAN-1)
 * if getrlimit fails to give us the max fd.
 * We believe that it should only enough to scan the first 100 fds.
 */
#define MAX_FD_TO_SCAN 100

class BasicEntry {
protected:
  unsigned int rc_;

public:
  /**
   * Constructor
   */
  BasicEntry();

  /**
   * Increment the reference count by 1.
   */
  void increment_rc();

  /**
   * Decrement the reference count by 1.
   * Return the reference count after decrement
   */
  unsigned int decrement_rc();
};

class UmaskEntry : public BasicEntry{
private:
  mode_t umask_;

public:
  /**
   * Constructor
   */
  UmaskEntry(mode_t m);

  /**
   * Return mask
   */
  mode_t get_umask();

  /**
   * This function will update mask value to the given mask.
   */
  void set_umask(mode_t m);
};

class FileDescriptorEntry {
private:
  int fd_;
  int flags_;

public:
  /**
   * Constructor
   */
  FileDescriptorEntry(int fd, int flags);

  /**
   * Copy Constructor
   */
  FileDescriptorEntry(FileDescriptorEntry& fd_entry);

  /**
   * Return file descriptor
   */
  int get_fd();

  /**
   * This function will update file descriptor value to the given fd.
   */
  void set_fd(int fd);

  /**
   * Return file descriptor flags
   */
  int get_flags();

  /**
   * This function will update file descriptor flags value to the given flags.
   */
  void set_flags(int flags);

  /**
   * Get string representation of this FileDescriptorEntry object
   */
  std::string to_string();
};

class FileDescriptorTableEntry : public BasicEntry{
private:
  // <traced fd, replayed FileDescriptorEntry>
  std::map<int, FileDescriptorEntry*> fd_table_;

public:
  /**
   * Basic Constructor
   */
  FileDescriptorTableEntry();

  /**
   * Copy Constructor
   */
  FileDescriptorTableEntry(FileDescriptorTableEntry& fd_table_entry);

  /**
   * Destructor that frees all the memory used by this table.
   */
  ~FileDescriptorTableEntry();

  /**
   * Return fd table
   */
  std::map<int, FileDescriptorEntry*>& get_fd_table();

  /**
   * Add a fd entry to the fd table.
   */
  void add_fd_entry(int traced_fd, int replayed_fd, int flags);

  /**
   * Remove file descriptor entry from the table.
   * Return -1 if traced_fd is invalid, otherwise, return
   * replayed fd for caller to close.
   */
  int remove_fd_entry(int traced_fd);

  /**
   * Get replayed fd for the given traced fd
   * Return -1 if traced_fd is invalid.
   */
  int get_fd(int traced_fd);

  /**
   * Determine whether there is a file descriptor entry for traced fd.
   * Return true if there is, return false otherwise.
   */
  bool has_fd(int traced_fd);

  /**
   * Get all traced fds for this process.
   */
  std::unordered_set<int> get_all_traced_fds();

  /**
   * Get all replayed fds in this process.
   */
  std::unordered_set<int> get_all_replayed_fds();

  /**
   * Update the replayed that is mapped to traced_fd
   */
  void update_fd(int traced_fd, int replayed_fd);

  /**
   * Get fd flags for the given traced fd
   */
  int get_flags(int traced_fd);

  /**
   * Set fd flags for the given traced fd to the given flags argument
   */
  void set_flags(int traced_fd, int flags);

  /**
   * Get string representation of this FileDescriptorTableEntry object
   */
  std::string to_string();
};

// <pid, FileDescriptorTableEntry>
typedef std::map<pid_t, FileDescriptorTableEntry*> PerPidFileDescriptorTableMap;
// <pid, umask entry>
typedef std::map<pid_t, UmaskEntry*> UmaskTable;

class ReplayerResourcesManager {
  /*
   * Note:
   * 1. fds are int because fds can be negative. Ex: FDCWD == -100
   * 2. flags are also int because flags in open man page is int.
   */

private:
  PerPidFileDescriptorTableMap fd_table_map_;
  UmaskTable umask_table_;
  SystemCallTraceReplayLogger *logger_;
  // Cached currently in-used fds in replayer
  std::unordered_set<int> replayer_used_fds_;

public:
  /**
   * Constructor
   */
  ReplayerResourcesManager();

  /**
   * This function needs to be called first before
   * using any method in this class for the following reason:
   * when sys call replayer starts, there is no fd table
   * for any process. We need to create the fd table for
   * the first process before replaying.
   */
  void initialize(SystemCallTraceReplayLogger *logger, pid_t pid, std::map<int, int>& fd_map);

  /**
   * This function will create a file descriptor entry for
   * traced fd and adds that entry to the correct fd table.
   */
  void add_fd(pid_t pid, int traced_fd, int replayed_fd, int flags);

  /**
   * This function will determine whether there is a file descriptor
   * entry for traced fd.
   * Return true if there is, return false otherwise.
   */
  bool has_fd(pid_t pid, int traced_fd);

  /**
   * This function generates a fd that is currently
   * unused in replayer.
   * Return a fd that is currently unused.
   */
  int generate_unused_fd(pid_t pid);

  /**
   * This function will return replayed file descriptor that corrsponds
   * to given traced file descriptor. Return -1 if traced_fd
   * is invalid.
   */
  int get_fd(pid_t pid, int traced_fd);

  /**
   * This function will return all traced file descriptors in
   * the given process.
   */
  std::unordered_set<int> get_all_traced_fds(pid_t pid);

  /**
   * This function will update the replayed file descriptor
   * for the given traced_fd.
   */
  void update_fd(pid_t pid, int traced_fd, int replayed_fd);

  /**
   * This function will remove the file descriptor entry
   * from pid fd table.
   *
   * Return a fd to close if traced_fd is valid, otherweise,
   * return -1.
   */
  int remove_fd(pid_t pid, int traced_fd);

  /**
   * This function will return flags of a file descriptor.
   */
  int get_flags(pid_t pid, int traced_fd);

  /**
   * This function will set file descriptor flags to the argument flags.
   */
  void set_flags(pid_t pid, int traced_fd, int flags);

  /**
   * This function will add flags to the file descriptor flags.
   */
  void add_flags(pid_t pid, int traced_fd, int flags);

  /**
   * This function will clone file descriptor table for a process.
   * If shared is true, increment the reference count for the fd table
   * without creating a new fd table for pid. This means
   * that they both share same fd table. If shared is false,
   * a new fd table will be created and the content of new
   * fd table is same as ppid fd table.
   */
  void clone_fd_table(pid_t ppid, pid_t pid, bool shared);

  /**
   * This function will destroy a file descriptor table
   * for a process.
   * It will decrement the reference count for pid fd table
   * and remove the pid fd table entry in the map.
   * It will only destroy fd table and return
   * a list of fds for replayer to close if the reference count
   * of fd table reaches to 0.
   * Return a list of fd for replayer to close
   */
  std::unordered_set<int> remove_fd_table(pid_t pid);

  /**
   * print_fd_manager() function will print out the
   * content of file descriptor manager. This is very useful
   * for debug.
   */
  void print_fd_manager();

  /**
   * Return umask value for a particular process
   */
  mode_t get_umask(pid_t pid);

  /**
   * Set umask value for a particular process
   */
  void set_umask(pid_t pid, mode_t mode);

  /**
   * Clone mask value for a particular process given its parent process
   * If shared is True, then two processes same mask entry, otherwise,
   * this function will create a new entry for pid.
   * pid is child process pid
   * ppid is parent process pid
   */
  void clone_umask(pid_t ppid, pid_t pid, bool shared);

  /**
   * Decrement the mask reference count for the given pid.
   * Only remove the entry if rc reaches to 0.
   */
  void remove_umask(pid_t pid);

  /**
   * Scan all fds in the replayer and see if there is any fd that
   * is not known to this manager. Print a warning message to indicate
   * this situation.
   * Note: we can periodically call this function to validate the state of our replayer,
   * so we know that every fd that is open is known to this resource manager.
   */
  void validate_consistency();

  /**
   * Check to see whether the given fd is currently used in the replayer.
   * Return true if it is currently in-used, false otherwise.
   */
  bool is_fd_in_use(int fd);
};
#endif /* REPLAYER_RESOURCES_MANAGER_HPP */
