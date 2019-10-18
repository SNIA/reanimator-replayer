// Copyrigth FSL Stony Brook

#ifndef VIRTUAL_ADDRESS_SPACE
#define VIRTUAL_ADDRESS_SPACE

#include <sys/types.h>
#include <cstdint>
#include <queue>

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
  std::priority_queue<VM_node> vma;

 public:
  VM_area() {}
  /*
   * find method is responsible for finding
   * VM_node which contains the addr
   */
  VM_node *find_VM_node(void *addr);

  /*
    delete
  */
  bool delete_VM_node(VM_node *node);

  /*
    inserts vm node to vma in sorted manner
  */

  void insert_VM_node(VM_node *node);
};

class VM_mamager {
 private:
  static VM_mamager *instance;
  VM_mamager() {}

 public:
  static VM_mamager *getInstance();

  /*
   * gets the virtual address space for
   * particular process
   */
  VM_area get_VM_area(pid_t pid);
};

#endif
