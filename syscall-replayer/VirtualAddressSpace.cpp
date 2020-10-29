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
 */

#include "VirtualAddressSpace.hpp"
#include <algorithm>
#include <iostream>
#include "SystemCallTraceReplayModule.hpp"

VM_manager* VM_manager::instance = NULL;
VM_manager* VM_manager::getInstance() {
  if (!instance) {
    instance = new VM_manager();
  }
  return instance;
}

VM_area* VM_manager::get_VM_area(pid_t pid) {
  std::unordered_map<pid_t, VM_area*>::const_iterator process_vm_area =
      process_map.find(pid);
  if (process_vm_area == process_map.end()) {
    VM_area* area = new VM_area();
    process_map[pid] = area;
    return area;
  }
  return process_map[pid];
}

void VM_area::insert_VM_node(VM_node* node) {
  vma_lock.lock();
  vma.push_back(node);
  vma_lock.unlock();
  return;
}

enum overlaps { left = 4, right = 2, enclosed = 8, enclosing = 1, invalid = 0 };

bool check_left_overlapping(uint64_t addr_int, size_t size, uint64_t start_addr,
                            uint64_t end_addr) {
  return ((addr_int + size) >= end_addr) && (addr_int > start_addr) &&
         (addr_int < end_addr);
}

bool check_right_overlapping(uint64_t addr_int, size_t size,
                             uint64_t start_addr, uint64_t end_addr) {
  return ((addr_int + size) > start_addr) && ((addr_int + size) < end_addr) &&
         (addr_int <= start_addr);
}

bool check_enclosed_overlapping(uint64_t addr_int, size_t size,
                                uint64_t start_addr, uint64_t end_addr) {
  return (addr_int <= start_addr) && ((addr_int + size) >= end_addr);
}

bool check_enclosing_overlapping(uint64_t addr_int, size_t size,
                                 uint64_t start_addr, uint64_t end_addr) {
  return (addr_int > start_addr) && ((addr_int + size) < end_addr);
}

std::vector<VM_node*>* VM_area::find_VM_node(void* addr, size_t size) {
  auto addr_int = reinterpret_cast<uint64_t>(addr);
  auto result = new std::vector<VM_node*>(vma.size());
  std::copy_if(vma.begin(), vma.end(), result->begin(), [&](VM_node* vnode)
                                                            -> bool {
    auto start_addr = reinterpret_cast<uint64_t>(vnode->traced_start_address);
    auto end_addr = start_addr + vnode->map_size;
    return check_left_overlapping(addr_int, size, start_addr, end_addr) ||
           check_right_overlapping(addr_int, size, start_addr, end_addr) ||
           check_enclosed_overlapping(addr_int, size, start_addr, end_addr) ||
           check_enclosing_overlapping(addr_int, size, start_addr, end_addr);
  });
  return result;
}

bool VM_area::delete_VM_node(void* addr, size_t size) {
  auto addr_int = reinterpret_cast<uint64_t>(addr);

  vma_lock.lock();

  list();
  auto result = find_VM_node(addr, size);

  for (auto node : *result) {
    if (node == NULL) continue;
    SystemCallTraceReplayModule::syscall_logger_->log_info(
        "addr ",
        boost::format("%02x") %
            reinterpret_cast<uint64_t>(node->traced_start_address));
    // we have only two cases
    // first delete the node
    // second just update
    auto start_addr = reinterpret_cast<uint64_t>(node->traced_start_address);
    auto end_addr = start_addr + node->map_size;

    int overlapped = 0;
    overlapped |= check_left_overlapping(addr_int, size, start_addr, end_addr)
                      ? left
                      : invalid;
    overlapped |= check_right_overlapping(addr_int, size, start_addr, end_addr)
                      ? right
                      : invalid;
    overlapped |=
        check_enclosed_overlapping(addr_int, size, start_addr, end_addr)
            ? enclosed
            : invalid;
    overlapped |=
        check_enclosing_overlapping(addr_int, size, start_addr, end_addr)
            ? enclosing
            : invalid;
    SystemCallTraceReplayModule::syscall_logger_->log_info("overllaped ",
                                                           overlapped);

    switch (overlapped) {
      case enclosed: {
        vma.erase(std::find(vma.begin(), vma.end(), node));
        break;
      }
      case left: {
        node->map_size = addr_int - start_addr;
        break;
      }
      case right: {
        auto new_start_addr = addr_int + size;
        node->replayed_start_address = reinterpret_cast<void*>(
            reinterpret_cast<uint64_t>(node->replayed_start_address) +
            (new_start_addr - start_addr));
        node->map_size -= (new_start_addr - start_addr);
        node->traced_start_address = reinterpret_cast<void*>(new_start_addr);
        break;
      }
      case enclosing: {
        // create a new node
        auto new_traced_addr = addr_int + size;
        auto new_replyed_addr =
            reinterpret_cast<uint64_t>(node->replayed_start_address) +
            new_traced_addr - start_addr;
        auto new_size = end_addr - new_traced_addr;
        auto new_node =
            new VM_node(reinterpret_cast<void*>(new_traced_addr),
                        reinterpret_cast<void*>(new_replyed_addr), new_size,
                        node->traced_fd, node->replayed_fd);
        vma.push_back(new_node);

        // update the size of the node
        node->map_size = addr_int - start_addr;
        break;
      }
    }
  }
  result->clear();
  delete result;

  vma_lock.unlock();
  return false;
}

void VM_area::list() {
  for (VM_node* node : vma) {
    SystemCallTraceReplayModule::syscall_logger_->log_info(
        "Current node in VM_area: traced_addr(",
        boost::format("0x%02x") % (node->traced_start_address), "), ",
        "replayed_addr(",
        boost::format("0x%02x") % (node->replayed_start_address), "), ",
        "size(", boost::format("0x%02x") % (node->map_size), "), ",
        "traced_fd(", boost::format("0x%02x") % (node->traced_fd), "), ",
        "replayed_fd(", boost::format("0x%02x") % (node->replayed_fd), ")");
  }
}
