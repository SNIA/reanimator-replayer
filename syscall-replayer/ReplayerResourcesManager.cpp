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
 * This file implements all the functions in the ReplayerResourcesManager
 * header file.
 *
 * Read ReplayerResourcesManager.hpp for more information about this class.
 */

#include "ReplayerResourcesManager.hpp"

ReplayerResourcesManager::ReplayerResourcesManager() {
}

void ReplayerResourcesManager::initialize(pid_t pid, std::map<int, int>& fd_map) {
  for (std::map<int, int>::iterator it = fd_map.begin();
    it != fd_map.end(); ++it) {
    int traced_fd = it->first;
    int replayed_fd = it->second;
    // Flags is 0
    fd_table_map_[pid][traced_fd] = std::make_pair(replayed_fd, 0);
    fd_rc_[traced_fd]++;
  }
}

void ReplayerResourcesManager::add_fd(pid_t pid, int traced_fd,
  int replayed_fd, int flags) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  FileDescriptorTable& fd_table = fd_table_map_[pid];
  fd_table[traced_fd] = std::make_pair(replayed_fd, flags);
  /* [] operator adds an entry for us if traced_fd is new in our map
   * and initialize the reference count to be 0. So all we have to do is increment the
   * reference count.
   * The following code can lead to traced fd -1 to have many reference counts.
   * This case is a valid one.
   */
  fd_rc_[traced_fd]++;
}

std::pair<bool, int> ReplayerResourcesManager::remove_fd(pid_t pid, int traced_fd) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
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
    fd_rc_.erase(traced_fd);
    return std::make_pair(true, replayed_fd);
  }
  return std::make_pair(false, 0);
}

int ReplayerResourcesManager::get_fd(pid_t pid, int traced_fd) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  // Get the corresponding fd table
  FileDescriptorTable& fd_table = fd_table_map_[pid];
  if (fd_table.find(traced_fd) == fd_table.end()) {
    return -1;
  }
  return fd_table[traced_fd].first;
}

int ReplayerResourcesManager::get_flags(pid_t pid, int traced_fd) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  FileDescriptorTable& fd_table = fd_table_map_[pid];
  return fd_table[traced_fd].second;
}

void ReplayerResourcesManager::add_flags(pid_t pid, int traced_fd, int flags) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  FileDescriptorTable& fd_table = fd_table_map_[pid];
  fd_table[traced_fd].second |= flags;
}

void ReplayerResourcesManager::remove_flags(pid_t pid, int traced_fd, int flags) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  FileDescriptorTable& fd_table = fd_table_map_[pid];
  fd_table[traced_fd].second &= ~flags;
}

int ReplayerResourcesManager::clone_fd_table(pid_t ppid, pid_t pid) {
  assert(fd_table_map_.find(ppid) != fd_table_map_.end());
  // Make a copy
  fd_table_map_[pid] = fd_table_map_[ppid];

  for (FileDescriptorTable::iterator iter = fd_table_map_[pid].begin();
    iter != fd_table_map_[pid].end(); iter++) {
    int traced_fd = iter->first;
    assert(fd_rc_.find(traced_fd) != fd_rc_.end());
    assert(fd_rc_[traced_fd] <= 0);
    // Increment reference count
    fd_rc_[traced_fd]++;
  }
  return 0;
}

void ReplayerResourcesManager::update_umask(pid_t pid, mode_t umask) {
  assert(umask_table_.find(pid) != umask_table_.end());
  // Update umask
  umask_table_[pid] = umask;
}

int ReplayerResourcesManager::clone_umask_table(pid_t ppid, pid_t pid) {
  assert(umask_table_.find(ppid) != umask_table_.end());
  // Make a copy
  umask_table_[pid] = umask_table_[ppid];
  return 0;
}

int ReplayerResourcesManager::remove_umask_table(pid_t pid) {
  assert(umask_table_.find(pid) != umask_table_.end());
  // Make a copy
  umask_table_.erase(pid);
  return 0;
}

std::vector<std::pair<bool, int>> ReplayerResourcesManager::remove_fd_table(pid_t pid) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
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

void ReplayerResourcesManager::print_fd_manager() {
  std::cout << "=====================================================================" << std::endl;
  std::cout << "---------------------- File Descriptor Manager ----------------------" << std::endl;
  // Print file desriptor tables
  for (PerPidFileDescriptorTableMap::iterator map_iter = fd_table_map_.begin();
    map_iter != fd_table_map_.end(); map_iter++) {
    pid_t pid = map_iter->first;
    std::cout << "Pid: " << pid << std::endl;
    for (FileDescriptorTable::iterator table_iter = map_iter->second.begin();
      table_iter != map_iter->second.end(); table_iter++) {
      int traced_fd = table_iter->first;
      FileDescriptor fd_info = table_iter->second;
      std::cout << "Traced fd: " << traced_fd;
      std::cout << " -> <Replayed fd: " << fd_info.first << ", flags: " << fd_info.second << ">" << std::endl;
    }
  }
  // Print reference count table
  std::cout << "---------------------- Reference Count ------------------------------" << std::endl;
  for (FileDescriptorReferenceCount::iterator iter = fd_rc_.begin();
    iter != fd_rc_.end(); iter++) {
    int traced_fd = iter->first;
    int reference_count = iter->second;
    std::cout << "Traced fd: " << traced_fd << ", Reference count: " << reference_count << std::endl;
  }
}
