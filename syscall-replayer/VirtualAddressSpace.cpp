// Copyright FSL Stony Brook

#include "VirtualAddressSpace.hpp"
#include <algorithm>

VM_mamager* VM_mamager::instance = NULL;
VM_mamager* VM_mamager::getInstance() {
  if (!instance) {
    instance = new VM_mamager();
  }
  return instance;
}

std::vector<VM_node*>* VM_area::find_VM_node(void* addr) {
  auto addr_int = reinterpret_cast<uint64_t>(addr);
  auto result = new std::vector<VM_node*>();
  std::copy_if(vma.begin(), vma.end(), result->begin(),
               [&](VM_node* vnode) -> bool {
                 auto start_addr =
                     reinterpret_cast<uint64_t>(vnode->traced_start_address);
                 auto end_addr = start_addr + vnode->map_size;
                 return addr_int > start_addr && addr_int < end_addr;
               });
  return result;
}
