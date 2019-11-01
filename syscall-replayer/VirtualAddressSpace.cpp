// Copyright FSL Stony Brook

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
  // if there isn't an existing mapping
  std::unordered_map<pid_t, VM_area*>::const_iterator got =
      process_map.find(pid);
  if (got == process_map.end()) {
    VM_area* area = new VM_area();
    // process_map.insert(pid, area);
    process_map[pid] = area;
    return area;
  }
  return process_map.at(pid);
}

// insert new VM_node to existing vm area
void VM_area::insert_VM_node(VM_node* node) {
  vma_lock.lock();
  vma.push_back(node);
  vma_lock.unlock();
  return;
}

std::vector<VM_node*>* VM_area::find_VM_node(void* addr) {
  auto addr_int = reinterpret_cast<uint64_t>(addr);
  auto result = new std::vector<VM_node*>();
  std::copy_if(vma.begin(), vma.end(), result->begin(),
               [&](VM_node* vnode) -> bool {
                 auto start_addr =
                     reinterpret_cast<uint64_t>(vnode->traced_start_address);
                 auto end_addr = start_addr + vnode->map_size;
                 return (addr_int > start_addr) && (addr_int < end_addr);
               });
  return result;
}

// find the left overlapping nodes
std::vector<VM_node*>* VM_area::find_left_overlapping_target(void* addr,
                                                             size_t size) {
  auto addr_int = reinterpret_cast<uint64_t>(addr);
  auto result = new std::vector<VM_node*>();
  std::copy_if(vma.begin(), vma.end(), result->begin(),
               [&](VM_node* vnode) -> bool {
                 auto start_addr =
                     reinterpret_cast<uint64_t>(vnode->traced_start_address);
                 auto end_addr = start_addr + vnode->map_size;
                 return ((addr_int + size) > end_addr) &&
                        (addr_int > start_addr) && (addr_int < end_addr);
               });
  return result;
}

// find the right overlapping nodes
std::vector<VM_node*>* VM_area::find_right_overlapping_target(void* addr,
                                                              size_t size) {
  auto addr_int = reinterpret_cast<uint64_t>(addr);
  auto result = new std::vector<VM_node*>();
  std::copy_if(
      vma.begin(), vma.end(), result->begin(), [&](VM_node* vnode) -> bool {
        auto start_addr =
            reinterpret_cast<uint64_t>(vnode->traced_start_address);
        auto end_addr = start_addr + vnode->map_size;
        return ((addr_int + size) > start_addr) &&
               ((addr_int + size) < end_addr) && (addr_int < start_addr);
      });
  return result;
}

// find all the nodes that need to be totally deleted (enclosed within the
// target region)
std::vector<VM_node*>* VM_area::find_enclosed_target(void* addr, size_t size) {
  auto addr_int = reinterpret_cast<uint64_t>(addr);
  auto result = new std::vector<VM_node*>();
  std::copy_if(
      vma.begin(), vma.end(), result->begin(), [&](VM_node* vnode) -> bool {
        auto start_addr =
            reinterpret_cast<uint64_t>(vnode->traced_start_address);
        auto end_addr = start_addr + vnode->map_size;
        return (addr_int <= start_addr) && ((addr_int + size) >= end_addr);
      });
  return result;
}

// find all the nodes that enclose the target region (cut in the middle)
std::vector<VM_node*>* VM_area::find_enclosing_target(void* addr, size_t size) {
  auto addr_int = reinterpret_cast<uint64_t>(addr);
  auto result = new std::vector<VM_node*>();
  std::copy_if(vma.begin(), vma.end(), result->begin(),
               [&](VM_node* vnode) -> bool {
                 auto start_addr =
                     reinterpret_cast<uint64_t>(vnode->traced_start_address);
                 auto end_addr = start_addr + vnode->map_size;
                 return addr_int > start_addr && (addr_int + size) < end_addr;
               });
  return result;
}

bool VM_area::delete_VM_node(void* addr, size_t size) {
  // find vm regions that need to be modifieda
  auto addr_int = reinterpret_cast<uint64_t>(addr);
  std::vector<VM_node*>* left_overlap =
      find_left_overlapping_target(addr, size);
  std::vector<VM_node*>* right_overlap =
      find_right_overlapping_target(addr, size);
  std::vector<VM_node*>* enclosing = find_enclosing_target(addr, size);
  std::vector<VM_node*>* enclosed = find_enclosed_target(addr, size);

  int overlapped = 0;

  vma_lock.lock();
  // update accordingly
  if (left_overlap->size() > 0) {
    for (VM_node* node : *left_overlap)
      node->map_size =
          addr_int - reinterpret_cast<uint64_t>(node->traced_start_address);
    overlapped++;
    SystemCallTraceReplayModule::syscall_logger_->log_info("left overlapped");
  }

  if (right_overlap->size() > 0) {
    for (VM_node* node : *right_overlap) {
      node->map_size = node->map_size -
                       (addr_int + size -
                        reinterpret_cast<uint64_t>(node->traced_start_address));
      node->traced_start_address = (void*)(addr_int + size);
    }
    overlapped++;
    SystemCallTraceReplayModule::syscall_logger_->log_info("right overlapped");
  }

  if (enclosing->size() > 0) {
    for (VM_node* node : *enclosing) {
      vma.erase(std::find(vma.begin(), vma.end(), node));
      // Comment UMIT call this later delete node;
    }
    enclosing->clear();
    overlapped++;
    SystemCallTraceReplayModule::syscall_logger_->log_info(
        "enclosing overlapped");
  }

  if (enclosed->size() > 0) {
    for (VM_node* node : *enclosed) {
      // need to insert a new node
      size_t new_size =
          (reinterpret_cast<uint64_t>(node->traced_start_address) +
           node->map_size) -
          (reinterpret_cast<uint64_t>(addr) + size);

      VM_node* new_node =
          new VM_node((void*)(reinterpret_cast<uint64_t>(addr) + size), NULL,
                      new_size, node->traced_fd, node->replayed_fd);
      insert_VM_node(new_node);

      // update the existing node
      node->map_size = reinterpret_cast<uint64_t>(addr) -
                       reinterpret_cast<uint64_t>(node->traced_start_address);
    }
    overlapped++;
    SystemCallTraceReplayModule::syscall_logger_->log_info(
        "enclosed overlapped");
  }
  vma_lock.unlock();

  left_overlap->clear();
  right_overlap->clear();
  enclosing->clear();
  enclosed->clear();

  delete left_overlap;
  delete right_overlap;
  delete enclosing;
  delete enclosed;

  if (overlapped == 0)
    return false;
  else
    return true;
}
