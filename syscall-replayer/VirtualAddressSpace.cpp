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

bool check_left_overlapping(uint64_t addr_int, size_t size, uint64_t start_addr,
                            uint64_t end_addr) {
  return ((addr_int + size) > end_addr) && (addr_int > start_addr) &&
         (addr_int < end_addr);
}

bool check_right_overlapping(uint64_t addr_int, size_t size,
                             uint64_t start_addr, uint64_t end_addr) {
  return ((addr_int + size) > start_addr) && ((addr_int + size) < end_addr) &&
         (addr_int < start_addr);
}

bool check_enclosed_overlapping(uint64_t addr_int, size_t size,
                                uint64_t start_addr, uint64_t end_addr) {
  return (addr_int <= start_addr) && ((addr_int + size) >= end_addr);
}

bool check_enclosing_overlapping(uint64_t addr_int, size_t size,
                                 uint64_t start_addr, uint64_t end_addr) {
  return (addr_int >= start_addr) && ((addr_int + size) <= end_addr);
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
  vma_lock.lock();

  list();
  auto result = find_VM_node(addr, size);

  for (auto r : *result) {
    if (r != NULL) {
      SystemCallTraceReplayModule::syscall_logger_->log_info(
          "addr ",
          boost::format("%02x") %
              reinterpret_cast<uint64_t>(r->traced_start_address));
    }
  }

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
