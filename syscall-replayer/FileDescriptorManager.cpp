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
 * This file implements all the functions in the FileDescriptorManager
 * header file.
 *
 * Read FileDescriptorManager.hpp for more information about this class.
 */

#include "FileDescriptorManager.hpp"

FileDescriptorManager::FileDescriptorManager() {
}

void FileDescriptorManager::add_fd(pid_t pid, int traced_fd,
  int replayed_fd, int flags) {
  if (fd_table_map_.find(pid) == fd_table_map_.end()) {
    // LOG_INFO(__func__);
    abort();
  }
  FileDescriptorTable& fd_table = fd_table_map_[pid];
  fd_table[traced_fd] = std::make_pair(replayed_fd, flags);
  // [] operator adds an entry for us if traced_fd is new in our map
  // and initialize the reference count to be 0. So all we have to do is increment the
  // reference count.
  fd_rc_[traced_fd]++;
}

std::pair<bool, int> FileDescriptorManager::remove_fd(pid_t pid, int traced_fd) {
  if (fd_table_map_.find(pid) == fd_table_map_.end()) {
    // LOG_INFO(__func__);
    abort();
  }
  FileDescriptorTable& fd_table = fd_table_map_[pid];
  if (fd_rc_.find(traced_fd) == fd_rc_.end() || fd_table.find(traced_fd) == fd_table.end()) {
    return std::make_pair(false, -1);
  }
  if (fd_rc_[traced_fd] <= 0) {
    return std::make_pair(false, -1);
  }
  // Decrement the reference
  fd_rc_[traced_fd]--;
  // Get replayed fd
  int replayed_fd = fd_table[traced_fd].first;
  // Remove the fd entry
  fd_table.erase(traced_fd);
  if (fd_rc_[traced_fd] == 0) {
    return std::make_pair(true, replayed_fd);
  }
  return std::make_pair(false, 0);
}

int FileDescriptorManager::get_fd(pid_t pid, int traced_fd) {
  if (fd_table_map_.find(pid) == fd_table_map_.end()) {
    // LOG_INFO(__func__);
    abort();
  }
  // Get the corresponding fd table
  FileDescriptorTable& fd_table = fd_table_map_[pid];
  if (fd_table.find(traced_fd) == fd_table.end()) {
    return -1;
  }
  return fd_table[traced_fd].first;
}

void FileDescriptorManager::add_flags(pid_t pid, int traced_fd, int flags) {
  FileDescriptorTable& fd_table = fd_table_map_[pid];
  fd_table[traced_fd].second |= flags;
}

void FileDescriptorManager::remove_flags(pid_t pid, int traced_fd, int flags) {
  FileDescriptorTable& fd_table = fd_table_map_[pid];
  fd_table[traced_fd].second &= (!flags);
}

int FileDescriptorManager::clone_fd_table(pid_t ppid, pid_t pid) {
  if (fd_table_map_.find(ppid) == fd_table_map_.end()) {
    // LOG_INFO(__func__);
    abort();
  }
  // Make a copy
  FileDescriptorTable p_fd_table = fd_table_map_[ppid];
  fd_table_map_[pid] = p_fd_table;

  for (FileDescriptorTable::iterator iter = p_fd_table.begin();
    iter != p_fd_table.end(); iter++) {
    int traced_fd = iter->first;
    if (fd_rc_.find(traced_fd) == fd_rc_.end()) {
      return -1;
    }
    // Increment reference count
    fd_rc_[traced_fd]++;
  }
  return 0;
}

std::vector<std::pair<bool, int>> FileDescriptorManager::remove_fd_table(pid_t pid) {
  if (fd_table_map_.find(pid) == fd_table_map_.end()) {
    // LOG_INFO(__func__);
    abort();
  }
  // Get the corresponding fd table
  FileDescriptorTable& fd_table = fd_table_map_[pid];
  std::vector<std::pair<bool, int>> fds;
  for (FileDescriptorTable::iterator iter = fd_table.begin();
  	iter != fd_table.end(); iter++) {
  	int traced_fd = iter->first;
    std::pair<bool, int> fd_info = remove_fd(pid, traced_fd);

    fds.push_back(fd_info);
  }
  fd_table_map_.erase(pid);
  return fds;
}
