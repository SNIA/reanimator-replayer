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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

ReplayerResourcesManager::ReplayerResourcesManager() {
}

void ReplayerResourcesManager::initialize(SystemCallTraceReplayLogger *logger,
  pid_t pid, std::map<int, int>& fd_map) {
  logger_ = logger;
  // Create a new UmaskEntry for trace application
  umask_table_[pid] = new UmaskEntry(0);

  fd_table_map_[pid] = new FileDescriptorTableEntry();
  for (std::map<int, int>::iterator iter = fd_map.begin();
    iter != fd_map.end(); ++iter) {
    int traced_fd = iter->first;
    int replayed_fd = iter->second;
    // Flags is 0
    add_fd(pid, traced_fd, replayed_fd, 0);
  }

  /*
   * We have to scan for the open fds once, at startup time.
   * This is necessary because we need it to genereate correct
   * unused fds since dup2 needs an unused fd in replayer.
   * This will be a very fast scan because lseek is a purely in-kernel call
   * it looks up the fd (failing right then if the fd is closed, which will be true for over 90% of the scanned values)
   * and then accesses the file table entry associated with the descriptor (following a couple of pointers).
   * The time will be dominated by the cost of entering and exiting the kernel.
   * To be safe, we scan all the way to MAX_FDs.
   */
  struct rlimit rlim;
  int result;
  // If getrlimit fails, we will scan only first 100 fds.
  int max_fds = 100;
  // Get the limit on open files
  result = getrlimit(RLIMIT_NOFILE, &rlim);
  if (result >= 0) {
    // getrlimit succeeds, sow we will scan all fds.
    max_fds = rlim.rlim_cur;
  }
  std::unordered_set<int> known_fds = fd_table_map_[pid]->get_all_fds();
  logger_->log_info("Start initial fd scan. Cache all fds that are not known to the resource manager.");
  for (int fd = 0; fd <= max_fds; fd++) {
    if (is_fd_in_use(fd) && known_fds.find(fd) == known_fds.end()) {
      replayer_used_fds_.insert(fd);
    }
  }
  if (replayer_used_fds_.size() > 1) {
    logger_->log_warn("Find multiple fds that are used by the replayer. Printing all of them");
    for (std::unordered_set<int>::iterator it = replayer_used_fds_.begin();
      it != replayer_used_fds_.end();
      it++) {
      logger_->log_warn(*it);
    }
  }
}

void ReplayerResourcesManager::add_fd(pid_t pid, int traced_fd,
  int replayed_fd, int flags) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  fd_table_map_[pid]->add_fd_entry(traced_fd, replayed_fd, flags);
}

int ReplayerResourcesManager::get_fd(pid_t pid, int traced_fd) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  return fd_table_map_[pid]->get_fd(traced_fd);
}

bool ReplayerResourcesManager::has_fd(pid_t pid, int traced_fd) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  return fd_table_map_[pid]->has_fd(traced_fd);
}

int ReplayerResourcesManager::generate_unused_fd(pid_t pid) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  std::unordered_set<int> used_fds = fd_table_map_[pid]->get_all_fds();
  // Add cached replayer fds to used fds (ex. logger fd)
  used_fds.insert(replayer_used_fds_.begin(), replayer_used_fds_.end());
  int unused = 0;
  while (used_fds.find(unused) != used_fds.end()) {
    // Keep incrementing unused until we find a fd that is not used.
    unused++;
  }
  return unused;
}

void ReplayerResourcesManager::update_fd(pid_t pid, int traced_fd, int replayed_fd) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  fd_table_map_[pid]->update_fd(traced_fd, replayed_fd);
}

int ReplayerResourcesManager::get_flags(pid_t pid, int traced_fd) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  return fd_table_map_[pid]->get_flags(traced_fd);
}

void ReplayerResourcesManager::update_flags(pid_t pid, int traced_fd, int flags) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  fd_table_map_[pid]->update_flags(traced_fd, flags);
}

int ReplayerResourcesManager::remove_fd(pid_t pid, int traced_fd) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  FileDescriptorTableEntry* fd_table_ptr = fd_table_map_[pid];
  return fd_table_ptr->remove_fd_entry(traced_fd);
}

void ReplayerResourcesManager::clone_fd_table(pid_t ppid, pid_t pid, bool shared) {
  assert(fd_table_map_.find(ppid) != fd_table_map_.end());
  FileDescriptorTableEntry* p_fd_table_ptr = fd_table_map_[ppid];
  if (shared) {
    fd_table_map_[pid] = p_fd_table_ptr;
    p_fd_table_ptr->increment_rc();
  } else {
    // Make a copy
    fd_table_map_[pid] = new FileDescriptorTableEntry(*p_fd_table_ptr);
  }
}

std::unordered_set<int> ReplayerResourcesManager::remove_fd_table(pid_t pid) {
  assert(fd_table_map_.find(pid) != fd_table_map_.end());
  FileDescriptorTableEntry* fd_table_ptr = fd_table_map_[pid];
  // Decrement the reference count for this process's fd table.
  unsigned int rc = fd_table_ptr->decrement_rc();
  // If reference count reaches 0, we will destroy the fd table.
  std::unordered_set<int> fds;
  if (rc <= 0) {
    // Need to ask replayer to close all those fds
    fds = fd_table_ptr->get_all_fds();
    // Free the memory that is used by fd table
    delete fd_table_ptr;
  }
  // Remove the entry for pid.
  fd_table_map_.erase(pid);
  return fds;
}

void ReplayerResourcesManager::print_fd_manager() {
  logger_->log_info("=====================================================================");
  logger_->log_info("---------------------- File Descriptor Manager ----------------------");
  // Print file desriptor tables
  for (PerPidFileDescriptorTableMap::iterator map_iter = fd_table_map_.begin();
    map_iter != fd_table_map_.end(); map_iter++) {
    pid_t pid = map_iter->first;
    logger_->log_info("Pid: " + std::to_string(pid));
    FileDescriptorTableEntry *table = map_iter->second;
    logger_->log_info(table->to_string());
  }
}

mode_t ReplayerResourcesManager::get_umask(pid_t pid) {
  assert(umask_table_.find(pid) != umask_table_.end());
  return umask_table_[pid]->get_umask();
}

void ReplayerResourcesManager::set_umask(pid_t pid, mode_t mode) {
  assert(umask_table_.find(pid) != umask_table_.end());
  umask_table_[pid]->set_umask(mode);
}

void ReplayerResourcesManager::clone_umask(pid_t ppid, pid_t pid, bool shared) {
  assert(umask_table_.find(ppid) != umask_table_.end());
  // Check if two processes share same umask
  if (shared) {
    // Make pid points to same umask
    umask_table_[pid] = umask_table_[ppid];
    // Increment rc.
    umask_table_[pid]->increment_rc();
  } else {
    // Create a new UmaskEntry since they don't share umask.
    UmaskEntry *p_umask = umask_table_[ppid];
    umask_table_[pid] = new UmaskEntry(p_umask->get_umask());
  }
}

void ReplayerResourcesManager::remove_umask(pid_t pid) {
  assert(umask_table_.find(pid) != umask_table_.end());
  // Decrement the reference count for this process's umask.
  unsigned int rc = umask_table_[pid]->decrement_rc();
  // If reference count reaches 0, we will remove the entry from the table.
  if (rc <= 0) {
    // Free the memory that is used by UmaskEntry
    delete umask_table_[pid];
  }
  // Remove the entry from umask table.
  umask_table_.erase(pid);
}

void ReplayerResourcesManager::validate_consistency() {
  /*
   * This will be a very fast scan because lseek is a purely in-kernel call
   * it looks up the fd (failing right then if the fd is closed, which will be true for over 90% of
   * the scanned values) and then accesses the file table entry associated with the descriptor
   * (following a couple of pointers). The time will be dominated by the cost of entering
   * and exiting the kernel. To be safe, we scan all the way to MAX_FDs.
   */
  struct rlimit rlim;
  int result;
  int max_fds;
  // Get the limit on open files
  result = getrlimit(RLIMIT_NOFILE, &rlim);
  if (result >= 0) {
    // getrlimit succeeds, so we will scan all fds.
    max_fds = rlim.rlim_cur;
  } else {
    // If getrlimit fails, we will scan only first MAX_FD_TO_SCAN fds.
    max_fds = MAX_FD_TO_SCAN;
  }

  // Hold all fds that resource manager knows
  std::unordered_set<int> used_fds;
  // Add cached replayer fds to used fds (ex. logger fd)
  used_fds.insert(replayer_used_fds_.begin(), replayer_used_fds_.end());

  for (PerPidFileDescriptorTableMap::iterator iter = fd_table_map_.begin();
    iter != fd_table_map_.end();
    ++iter) {
    pid_t pid = iter->first;
    std::unordered_set<int> process_used_fds = fd_table_map_[pid]->get_all_fds();
    used_fds.insert(process_used_fds.begin(), process_used_fds.end());
  }

  for (int fd = 0; fd < max_fds; fd++) {
    bool fd_used = is_fd_in_use(fd);
    // Make every fd that is open is known to this resource manager.
    if (fd_used && used_fds.find(fd) == used_fds.end()) {
      /*
       * This fd is in use, but the resource manager doesn't know about that.
       * Let's print a warning message to warn the user about this situation.
       * This is caused by some code that creates a fd by opening
       * a file without the resource manager knowing it. This could
       * cause serious problems in replaying.
       */
      logger_->log_warn("Unknown and currently in used file descriptor to the resource manager \
        is found: fd #" + fd);
    } else if (!fd_used && used_fds.find(fd) != used_fds.end()) {
      /*
       * This fd is NOT in use, but the resource manager thinks that
       * this fd is in use. Let's print a warning message to warn the user
       * about this situation. This is caused by some code that closes a fd without the
       * resource manager knowing it. This could
       * cause serious problems in replaying.
       */
      logger_->log_warn("Unused file descriptor, but the resource manager thinks it is in used: fd #" + fd);
    }

    /*
     * Note that fd_used && used_fds.find(unused) != used_fds.end() is
     * okay because this condition indicates that fd is used and resource
     * manager knows about it.
     * !fd_used && used_fds.find(unused) == used_fds.end() is also
     * okay because this condition indicates that fd is not used and the resource
     * manager doesn't know about it.
     */
  }
}

bool ReplayerResourcesManager::is_fd_in_use(int fd) {
  int result = lseek(fd, 0, SEEK_CUR);
  if (result >= 0 || EBADF != errno) {
    return true;
  }
  return false;
}

// =========================== BasicEntry Implementation ==========================
BasicEntry::BasicEntry():rc_(1) { }

void BasicEntry::increment_rc() {
  rc_++;
}

unsigned int BasicEntry::decrement_rc() {
  assert(rc_ != 0);
  rc_--;
  return rc_;
}

// =========================== UmaskEntry Implementation ==========================
UmaskEntry::UmaskEntry(mode_t m):BasicEntry(), umask_(m) { }

mode_t UmaskEntry::get_umask() {
  return umask_;
}

void UmaskEntry::set_umask(mode_t m) {
  umask_ = m;
}

// =========================== FileDescriptorEntry Implementation ==========================
FileDescriptorEntry::FileDescriptorEntry(int fd, int flags):fd_(fd), flags_(flags) { }

FileDescriptorEntry::FileDescriptorEntry(FileDescriptorEntry& fd_entry) {
  fd_ = fd_entry.get_fd();
  flags_ = fd_entry.get_flags(); 
}

int FileDescriptorEntry::get_fd() {
  return fd_;
}

void FileDescriptorEntry::set_fd(int fd) {
  fd_ = fd;
}

int FileDescriptorEntry::get_flags() {
  return flags_;
}

void FileDescriptorEntry::set_flags(int flags) {
  flags_ = flags;
}

std::string FileDescriptorEntry::to_string() {
  std::stringstream ss;
  ss << "<Replayed fd: " << fd_ << ", flags: " << flags_ << ">";
  return ss.str();
}

// =========================== FileDescriptorTableEntry Implementation ==========================
FileDescriptorTableEntry::FileDescriptorTableEntry():BasicEntry() { }

FileDescriptorTableEntry::FileDescriptorTableEntry(FileDescriptorTableEntry& fd_table_entry):BasicEntry() {
  std::map<int, FileDescriptorEntry*>& fd_table_copy = fd_table_entry.get_fd_table();
  for (std::map<int, FileDescriptorEntry*>::iterator iter = fd_table_copy.begin();
    iter != fd_table_copy.end(); iter++) {
    int traced_fd = iter->first;
    FileDescriptorEntry* fd_ptr = iter->second;
    add_fd_entry(traced_fd, fd_ptr->get_fd(), fd_ptr->get_flags());
  }
}

FileDescriptorTableEntry::~FileDescriptorTableEntry() {
  for (std::map<int, FileDescriptorEntry*>::iterator iter = fd_table_.begin();
    iter != fd_table_.end(); iter++) {
    FileDescriptorEntry* fd_ptr = iter->second;
    delete fd_ptr;
  }
}

std::map<int, FileDescriptorEntry*>& FileDescriptorTableEntry::get_fd_table() {
  return fd_table_;
}

void FileDescriptorTableEntry::add_fd_entry(int traced_fd, int replayed_fd, int flags) {
  // fd_table_ shouldn't have an entry for traced_fd if traced_fd is NOT -1
  assert(traced_fd == -1 || fd_table_.find(traced_fd) == fd_table_.end());
  /*
   * Because an application can call open system call with invalid
   * file path many times and get -1 as its return value, this function
   * can be called many times with traced_fd being -1, we don't want
   * to have a memory leak problem where we keep creating a entry
   * for traced_fd=-1
   */
  if (traced_fd != -1 || !has_fd(traced_fd)) {
    // Create a FileDescriptorEntry
    fd_table_[traced_fd] = new FileDescriptorEntry(replayed_fd, flags);
  }
}

int FileDescriptorTableEntry::remove_fd_entry(int traced_fd) {
  /*
   * fd_table_ may not have an entry for traced_fd because
   * traced_fd is invalid.
   */
  if (!has_fd(traced_fd)) {
    return -1;
  }
  int replayed_fd = fd_table_[traced_fd]->get_fd();
  // Free the memory
  delete fd_table_[traced_fd];
  // Erase the entry
  fd_table_.erase(traced_fd);
  return replayed_fd;
}

int FileDescriptorTableEntry::get_fd(int traced_fd) {
  /*
   * fd_table_ may not have an entry for traced_fd because
   * traced_fd is invalid.
   */
  if (!has_fd(traced_fd)) {
    return -1;
  }
  // Return replayed fd corresponding to traced_fd
  return fd_table_[traced_fd]->get_fd();
}

bool FileDescriptorTableEntry::has_fd(int traced_fd) {
  if (fd_table_.find(traced_fd) == fd_table_.end()) {
    return false;
  } else {
    return true;
  }
}

std::unordered_set<int> FileDescriptorTableEntry::get_all_fds() {
  std::unordered_set<int> fds;
  for (std::map<int, FileDescriptorEntry*>::iterator iter = fd_table_.begin();
    iter != fd_table_.end(); iter++) {
    FileDescriptorEntry* fd_ptr = iter->second;
    fds.insert(fd_ptr->get_fd());
  }
  return fds;
}

void FileDescriptorTableEntry::update_fd(int traced_fd, int replayed_fd) {
  // fd_table_ should have an entry for traced_fd
  assert(fd_table_.find(traced_fd) != fd_table_.end());
  // Create a FileDescriptorEntry
  fd_table_[traced_fd]->set_fd(replayed_fd);
}

int FileDescriptorTableEntry::get_flags(int traced_fd) {
  // fd_table_ should have an entry for traced_fd
  assert(fd_table_.find(traced_fd) != fd_table_.end());
  // Create a FileDescriptorEntry
  return fd_table_[traced_fd]->get_flags();
}

void FileDescriptorTableEntry::update_flags(int traced_fd, int flags) {
  // fd_table_ should have an entry for traced_fd
  assert(fd_table_.find(traced_fd) != fd_table_.end());
  // Create a FileDescriptorEntry
  fd_table_[traced_fd]->set_flags(flags);
}

std::string FileDescriptorTableEntry::to_string() {
  std::stringstream ss;
  ss << "Reference count: " << rc_ << std::endl;
  for (std::map<int, FileDescriptorEntry*>::iterator table_iter = fd_table_.begin();
    table_iter != fd_table_.end();
    table_iter++) {
    int traced_fd = table_iter->first;
    FileDescriptorEntry* fd_entry = table_iter->second;
    ss << "Traced fd: " << traced_fd << " -> ";
    ss << fd_entry->to_string() << std::endl;
  }
  return ss.str();
}
