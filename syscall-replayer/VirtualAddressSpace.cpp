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

//insert new VM_node to existing vm area
void VM_area::insert_VM_node(VM_node *node){
  vma.push_back(node);
  return;
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
                 return (addr_int + size) > start_addr && (addr_int + size) < end_addr;
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

//find all the nodes that enclose the target region (cut in the middle)
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
  //find vm regions that need to be modifieda
  auto addr_int = reinterpret_cast<uint64_t>(addr);
  std::vector<VM_node*>* left_overlap = find_VM_node(addr);
  std::vector<VM_node*>* right_overlap = find_right_overlapping_target(addr, size);
  std::vector<VM_node*>* enclosing = find_enclosing_target(addr, size);
  std::vector<VM_node*>* enclosed = find_enclosed_target(addr, size);

  int overlapped = 0;

  //update accordingly
  if(left_overlap->size() > 0){
    for(VM_node* node: *left_overlap)
      node->map_size = addr_int - reinterpret_cast<uint64_t>(node->traced_start_address);
    overlapped++;
  }
  
  if(right_overlap->size() > 0){
    for(VM_node* node: *right_overlap){
      node->traced_start_address = addr;
      node->map_size = node->map_size - (addr_int - reinterpret_cast<uint64_t>(node->traced_start_address));
    }
    overlapped++;
  }

  //how do I delete from the actual vector??
  if(enclosing->size() > 0){
    for(VM_node* node: *enclosing){
      delete node;
    }
    enclosing->clear();
    overlapped++;
  }

  if(enclosed->size() > 0){
    for(VM_node* node: *enclosed){
      //need to insert a new node
      size_t new_size = (reinterpret_cast<uint64_t>(node->traced_start_address) + node->map_size) - (reinterpret_cast<uint64_t>(addr) + size);

      //my assumption is fd should be the same, need checking
      VM_node* new_node = new VM_node((void *)(reinterpret_cast<uint64_t>(addr) + size), NULL, new_size, node->traced_fd, node->replayed_fd);
      insert_VM_node(new_node);

      //update the existing node
      node->map_size = reinterpret_cast<uint64_t>(addr) - reinterpret_cast<uint64_t>(node->traced_start_address);
    }
    overlapped++;
}

if(overlapped == 0)
  return false;
else return true;


}
