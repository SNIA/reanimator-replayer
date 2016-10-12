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

void FileDescriptorManager::add_fd(int pid, int traced_fd,
  int replayed_fd, std::vector<std::string> flags) {
  FileDescriptorTable fd_table = fd_table_map[pid];
  fd_table[traced_fd] = make_pair(replayed_fd, flags);
  // [] operator adds an entry for us if traced_fd is new in our map
  // and initialize the reference count to be 0. So all we have to do is increment the
  // reference count.
  fd_rc[traced_fd]++;
}

void FileDescriptorManager::add_flag(int pid, int traced_fd,
  std::string flag) {
  FileDescriptorTable fd_table = fd_table_map[pid];
  fd_table[traced_fd].second.push_back(flag);
}

void FileDescriptorManager::add_flags(int pid, int traced_fd,
  std::vector<std::string> flags) {
  FileDescriptorTable fd_table = fd_table_map[pid];
  std::vector<std::string> fd_flags = fd_table[traced_fd].second;
  // Insert all the flags into fd_flag
  fd_flags.insert(fd_flags.end(), flags.begin(), flags.end());
}

void FileDescriptorManager::clone_fd_table(int ppid, int pid) {
  FileDescriptorTable p_fd_table = fd_table_map[ppid];
  fd_table_map[pid] = p_fd_table;
}

int FileDescriptorManager::remove_fd(int pid, int traced_fd) {
  // Get the corresponding fd table
  FileDescriptorTable fd_table = fd_table_map[pid];
  fd_rc[traced_fd]--;
  // Tell the caller whether to remove this entry or not.
  if (fd_rc[traced_fd] == 0) {
  	int replayed_fd = fd_table[traced_fd].first;
  	fd_table.erase(traced_fd);
  	return replayed_fd;
  } else {
  	return -1;
  }
}

std::vector<int> FileDescriptorManager::remove_fd_table(int pid) {
  // Get the corresponding fd table
  FileDescriptorTable fd_table = fd_table_map[pid];
  std::vector<int> closing_fds;
  for (FileDescriptorTable::iterator iter = fd_table.begin();
  	iter != fd_table.end(); iter++) {
  	int traced_fd = iter->first;
  	fd_rc[traced_fd]--;
  	if (fd_rc[traced_fd] == 0) {
  		FileDescriptor replayed_fd_info = iter->second;
  		closing_fds.push_back(replayed_fd_info.first);
  	}
  }
  return closing_fds;
}

void FileDescriptorManager::update(int pid, int traced_fd,
  int replayed_fd, std::vector<std::string> flags) {
  // Get the corresponding fd table
  FileDescriptorTable fd_table = fd_table_map[pid];
  fd_table[traced_fd] = make_pair(replayed_fd, flags);
}

int FileDescriptorManager::get_fd(int pid, int traced_fd) {
  // Get the corresponding fd table
  FileDescriptorTable fd_table = fd_table_map[pid];
  return fd_table[traced_fd].first;
}
