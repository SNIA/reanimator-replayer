// Copyright FSL Stony Brook

#include "VirtualAddressSpace.hpp"

VM_mamager* VM_mamager::instance = NULL;
VM_mamager* VM_mamager::getInstance() {
  if (!instance) {
    instance = new VM_mamager();
  }
  return instance;
}
