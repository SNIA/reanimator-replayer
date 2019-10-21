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

//can be used to find the left overlapping nodes
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

//find the right overlapping nodes
std::vector<VM_node*>* VM_area::find_right_overlapping_target(void* addr, size_t size) {
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


//find all the nodes that need to be totally deleted (enclosed within the target region)
std::vector<VM_node*>* VM_area::find_enclosed_target(void *addr, size_t size){
  auto addr_int = reinterpret_cast<uint64_t>(addr);
  auto result = new std::vector<VM_node*>();
  std::copy_if(vma.begin(), vma.end(), result->begin(), 
                [&](VM_node* vnode) -> bool{
                  auto start_addr =
                     reinterpret_cast<uint64_t>(vnode->traced_start_address);
                 auto end_addr = start_addr + vnode->map_size;
                 return addr_int < start_addr && (addr_int + size) > end_addr;
                });
  return result;
}

//find all the nodes that enclose the target region
std::vector<VM_node*>* VM_area::find_enclosing_target(void *addr, size_t size){
  auto addr_int = reinterpret_cast<uint64_t>(addr);
  auto result = new std::vector<VM_node*>();
  std::copy_if(vma.begin(), vma.end(), result->begin(), 
                [&](VM_node* vnode) -> bool{
                  auto start_addr =
                     reinterpret_cast<uint64_t>(vnode->traced_start_address);
                 auto end_addr = start_addr + vnode->map_size;
                 return addr_int > start_addr && (addr_int + size) < end_addr;
                });
  return result;
}


bool VM_area::delete_VM_node(void *addr, size_t size){
  //no overlapping
  std::vector<VM_node *>* target = find_VM_node(addr);
  //overlapping
  //std::vector<VM_node *>* target_overlapped = find_VM_node(addr+size);
  if(target == NULL || target->size() < 1)
    return false;
  //iterate through the list
  for(std::vector<VM_node *>::iterator it = target->begin(); it != target->end(); ++it){
    auto addr_int = reinterpret_cast<uint64_t>(addr);
    auto start_addr = reinterpret_cast<uint64_t>((*it)->traced_start_address);
    
    
  }

}
