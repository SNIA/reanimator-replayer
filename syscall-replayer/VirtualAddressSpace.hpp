// Copyrigth FSL Stony Brook

#ifndef VIRTUAL_ADDRESS_SPACE
#define VIRTUAL_ADDRESS_SPACE

#include <stdint.h>
#include <sys/types.h>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

class VM_node {
 public:
  void *traced_start_address;
  void *replayed_start_address;
  uint64_t map_size;
  int32_t traced_fd;
  int32_t replayed_fd;

  VM_node(void *t_start_addr, void *r_start_addr, uint64_t size, int32_t t_fd,
          int32_t r_fd)
      : traced_start_address(t_start_addr),
        replayed_start_address(r_start_addr),
        map_size(size),
        traced_fd(t_fd),
        replayed_fd(r_fd) {}
};

class VM_area {
 private:
  std::mutex vma_lock;
 
 public:
  VM_area() {}
  std::vector<VM_node *> vma;
  /*
   * find method is responsible for finding
   * VM_node which contains the addr
   */
  std::vector<VM_node *> *find_VM_node(void *addr, size_t size);

  /*
   * find the target to delete
   */
  std::vector<VM_node *> *find_enclosed_target(void *addr, size_t size);

  std::vector<VM_node *> *find_enclosing_target(void *addr, size_t size);

  std::vector<VM_node *> *find_left_overlapping_target(void *addr, size_t size);

  std::vector<VM_node *> *find_right_overlapping_target(void *addr,
                                                        size_t size);
  bool delete_VM_node(void *addr, size_t size);

  void insert_VM_node(VM_node *node);
};

class VM_manager {
 private:
  static VM_manager *instance;
  std::unordered_map<pid_t, VM_area *> process_map;
  VM_manager() {}

 public:
  static VM_manager *getInstance();

  /*
   * gets the virtual address space for
   * particular process
   */
  VM_area *get_VM_area(pid_t pid);
};

#endif
