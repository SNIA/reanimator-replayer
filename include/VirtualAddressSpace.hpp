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
  bool delete_VM_node(void *addr, size_t size);

  void insert_VM_node(VM_node *node);

  void list();
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
