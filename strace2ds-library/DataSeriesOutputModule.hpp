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
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program contains the DataSeries API's which converts the strace
 * trace to the DataSeries binary format. These API's are called from
 * the strace code which creates a new DataSeries file, sets extents and
 * field value, adds record to the DataSeries file and finally flush
 * extents to the output DataSeries file.
 *
 * INITALIZATION AND USAGE
 * The constructor of this class takes three arguments:
 * 1. tablefile: the path of table file which contains information about
 *		 extent name, field name and field type.
 * 2. xml directory: created using script from tablefile name.
 * 3. outputfile : name of the output dataseries file
 */

#ifndef DATA_SERIES_OUTPUT_MODULE_HPP
#define DATA_SERIES_OUTPUT_MODULE_HPP
#ifndef USE_ENUMS
#define USE_ENUMS
#include <cassert>
#include <cstring>
#include <string>

#include <atomic>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <utility>

#include <DataSeries/DataSeriesFile.hpp>
#include <DataSeries/DataSeriesModule.hpp>
#include <DataSeries/ExtentType.hpp>

#include <strace2ds.h>

#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/xattr.h>
#include <utime.h>
#include "strace2ds-enums.h"

class DataSeriesOutputModule;

#ifndef ST_VALID
// some systems support this flag of statfs() but it’s missing from statfs.h
#define ST_VALID 0x0020
#endif /* ST_VALID */

/*
 * Extent size used to be 4096 bytes, but changing it to
 * 4,194,304 bytes (4MB) reduced CPU percentage by almost 20%
 */
#define DEFAULT_EXTENT_SIZE 0x400000 /* 4MB */

#define LTTNG_DEFAULT_SYSCALL_NUM -100

/*pair<nullable, ExtentType>*/
typedef std::pair<bool, ExtentType::fieldType> config_table_entry_pair;

/* map<extentname, config_table_entry_pair **> */
typedef std::unordered_map<std::string, config_table_entry_pair **>
    config_table_type;

/* pair<DS Field, DS field type> */
typedef std::pair<void *, ExtentType::fieldType> ExtentFieldTypePair;

/* map<fieldName, <DS Field, DS field type>*/
typedef ExtentFieldTypePair FieldMap[MAX_SYSCALL_FIELDS];

/* map<syscallName, FieldMap> */
typedef std::unordered_map<std::string, FieldMap> ExtentMap;

// map<extent name, OutputModule>
typedef std::unordered_map<std::string, OutputModule *> OutputModuleMap;

// function pointer type for system call args map
typedef void (DataSeriesOutputModule::*SysCallArgsMapFuncPtr)(void **, long *,
                                                              void **);

// map<extent_name, SysCallArgsMapFuncPtr>
typedef std::unordered_map<std::string, SysCallArgsMapFuncPtr> FuncPtrMap;

// map<syscall_name, syscall_number>
typedef std::unordered_map<std::string, int> SyscallNameNumberMap;

extern unsigned nsyscalls;

class DataSeriesOutputModule {
 public:
  static bool true_;
  static bool false_;

  // A map of untraced syscalls number and their respective counts
  std::unordered_map<long, int> untraced_sys_call_counts_;

  // Constructor to set up all extents and fields
  DataSeriesOutputModule(std::ifstream &table_stream, const std::string xml_dir,
                         const char *output_file);

  // Register the record and field values into DS fields
  bool writeRecord(const char *extent_name, long *args,
                   void *common_fields[DS_NUM_COMMON_FIELDS], void **v_args);

  // Sets the ioctl_size_ variable for an Ioctl System Call
  void setIoctlSize(uint64_t size);

  // Gets the ioctl_size_ variable for an Ioctl System Call
  uint64_t getIoctlSize();

  /**
   * Gets the next record number
   */
  uint64_t getNextID();

  // Sets the clone_ctid_index_ variable for a Clone System Call
  void setCloneCTIDIndex(u_int ctid_index);

  // Gets the clone_ctid_index_ variable for a Clone System Call
  u_int getCloneCTIDIndex();

  // Destructor to delete the module
  ~DataSeriesOutputModule();

 private:
  OutputModuleMap modules_;
  /*
   * Since OutputModuleMap's value is of type OutputModule*, we create
   * a cache of type OutputModule*
   */
  OutputModule **modules_cache_;

  ExtentMap extents_;

  /*
   * it is map for optimizing for accessing field maps
   * we initialize in constructor and delete inside constructor too
   * that is why we can not access outside of the constructor
   */
  std::unordered_map<std::string, int> *field_enum_cache;

  /*
   * Since ExtentMap's value is of type FieldMap, we create a cache
   * of type FieldMap
   */
  FieldMap **extents_cache_;

  /* Sink is a wrapper for a DataSeries output file. */
  DataSeriesSink ds_sink_;
  config_table_type config_table_;
  /*
   * Since config_table_type's value is of type config_table_entry_pair,
   * we create a cache of type config_table_entry_pair
   */
  config_table_entry_pair ***config_table_cache_;

  std::atomic<uint64_t> record_num_;
  // ioctl_size_ is the size of a buffer passed to an ioctl system call
  uint64_t ioctl_size_;
  /*
   * clone_ctid_index_ is the index of the ctid argument passed to a clone
   * system call
   */
  u_int clone_ctid_index_;

  /*
   * Map which holds mapping of sys call name with the address
   * of its corresponding sys call args map function
   */
  FuncPtrMap func_ptr_map_;
  /*
   * Since FuncPtrMap's value is of type SysCallArgsMapFuncPtr, we create
   * a cache of type SysCallArgsMapFuncPtr
   */
  SysCallArgsMapFuncPtr *func_ptr_map_cache_;

  std::string field_names[MAX_SYSCALL_FIELDS];

  SyscallNameNumberMap syscall_name_num_map;

  // Disable copy constructor
  DataSeriesOutputModule(const DataSeriesOutputModule &);

  // Initialize config table
  void initConfigTable(std::ifstream &table_stream);

  // Add a extent(system call)
  void addExtent(const std::string &extent_name, ExtentSeries &series);

  // Add a field(system call arg) to the desired extent
  void addField(const std::string &extent_name, const std::string &field_name,
                void *field, const ExtentType::fieldType field_type);

  // Set corresponding DS field to the given value
  void setField(const ExtentFieldTypePair &extent_field_value_,
                void *field_value, int var32);

  // Set corresponding DS field to null
  void setFieldNull(const ExtentFieldTypePair &extent_field_value_);

  template <typename FieldType, typename ValueType>
  void doSetField(const ExtentFieldTypePair &extent_field_value_,
                  void *field_value);

  /*
   * Creates mapping of sys call name with the address
   * of it corresponding args map function
   */
  void initArgsMapFuncPtr();

  /*
   * Creates mapping of syscall name with the syscall
   * number
   */
  void initSyscallNameNumberMap();

  /*
   * name conversion for the syscalls that named differently
   */
  void syscall_name_conversion(std::string *extent_name);

  // Initializes all the caches with NULL values
  void initCache();

  // Returns the length for field of type variable32
  int getVariable32FieldLength(void **args_map, const int field_enum);

  // Initialize args map for given system call
  void initArgsMap(void **args_map, const char *extent_name);

  // Maps Close System Call <field, value> pairs
  void makeCloseArgsMap(void **args_map, long *args, void **v_args);

  // Maps Open System Call <field, value> pairs
  void makeOpenArgsMap(void **args_map, long *args, void **v_args);

  // Maps Openat System Call <field, value> pairs
  void makeOpenatArgsMap(void **args_map, long *args, void **v_args);

  // Processes individual flag and mode bits
  void process_Flag_and_Mode_Args(void **args_map, unsigned int &num, int value,
                                  int field_enum);

  /*
   * Maps individual flag value for Open system call to its corresponding
   * field name.
   */
  u_int processOpenFlags(void **args_map, u_int flag);

  // Maps individual mode bits of mode argument to its corresponding field name
  mode_t processMode(void **args_map, long *args, u_int offset);

  // Convert time from a timeval to a uint64_t in Tfracs
  uint64_t timeval_to_Tfrac(struct timeval tv);

  // Convert time from a long (seconds) to a uint64_t in Tfracs
  uint64_t sec_to_Tfrac(time_t time);

  // Convert time from a timespec to a uint64_t in Tfracs
  uint64_t timespec_to_Tfrac(struct timespec ts);

  // Maps Read System Call <field, value> pairs
  void makeReadArgsMap(void **args_map, long *args, void **v_args);

  // Maps MMapPread System Call <field, value> pairs
  void makeMmapPreadArgsMap(void **args_map, long *args, void **v_args);

  // Maps MMapPwwrite System Call <field, value> pairs
  void makeMmapPwriteArgsMap(void **args_map, long *args, void **v_args);

  // Maps Write System Call <field, value> pairs
  void makeWriteArgsMap(void **args_map, long *args, void **v_args);

  // Maps Chdir System Call <field, value> pairs
  void makeChdirArgsMap(void **args_map, long *args, void **v_args);

  // Maps Chdir System Call <field, value> pairs
  void makeChrootArgsMap(void **args_map, long *args, void **v_args);

  // Maps Rmdir System Call <field, value> pairs
  void makeRmdirArgsMap(void **args_map, long *args, void **v_args);

  // Maps Unlink System Call <field, value> pairs
  void makeUnlinkArgsMap(void **args_map, long *args, void **v_args);

  // Maps Unlinkat System Call <field, value> pairs
  void makeUnlinkatArgsMap(void **args_map, long *args, void **v_args);

  // Maps Mkdir System Call <field, value> pairs
  void makeMkdirArgsMap(void **args_map, long *args, void **v_args);

  // Maps Mkdirat System Call <field, value> pairs
  void makeMkdiratArgsMap(void **args_map, long *args, void **v_args);

  // Maps Creat System Call <field, value> pairs
  void makeCreatArgsMap(void **args_map, long *args, void **v_args);

  // Maps Chmod System Calls <field, value> pairs
  void makeChmodArgsMap(void **args_map, long *args, void **v_args);

  // Maps Umask System Calls <field, value> pairs
  void makeUmaskArgsMap(void **args_map, long *args, void **v_args);

  // Maps Setxattr System Calls <field, value> pairs
  void makeSetxattrArgsMap(void **args_map, long *args, void **v_args);

  // Maps LSetxattr System Calls <field, value> pairs
  void makeLSetxattrArgsMap(void **args_map, long *args, void **v_args);

  // Maps Getxattr System Calls <field, value> pairs
  void makeGetxattrArgsMap(void **args_map, long *args, void **v_args);

  // Maps LGetxattr System Calls <field, value> pairs
  void makeLGetxattrArgsMap(void **args_map, long *args, void **v_args);

  // Maps FSetxattr System Calls <field, value> pairs
  void makeFSetxattrArgsMap(void **args_map, long *args, void **v_args);

  // Maps FGetxattr System Calls <field, value> pairs
  void makeFGetxattrArgsMap(void **args_map, long *args, void **v_args);

  // Maps Listxattr System Calls <field, value> pairs
  void makeListxattrArgsMap(void **args_map, long *args, void **v_args);

  // Maps LListxattr System Calls <field, value> pairs
  void makeLListxattrArgsMap(void **args_map, long *args, void **v_args);

  // Maps FListxattr System Calls <field, value> pairs
  void makeFListxattrArgsMap(void **args_map, long *args, void **v_args);

  // Maps FLock System Calls <field, value> pairs
  void makeFLockArgsMap(void **args_map, long *args, void **v_args);

  // Maps Removexattr System Calls <field, value> pairs
  void makeRemovexattrArgsMap(void **args_map, long *args, void **v_args);

  // Maps LRemovexattr System Calls <field, value> pairs
  void makeLRemovexattrArgsMap(void **args_map, long *args, void **v_args);

  // Maps FRemovexattr System Calls <field, value> pairs
  void makeFRemovexattrArgsMap(void **args_map, long *args, void **v_args);

  // Maps FChmod System Calls <field, value> pairs
  void makeFChmodArgsMap(void **args_map, long *args, void **v_args);

  // Maps FChdir System Calls <field, value> pairs
  void makeFChdirArgsMap(void **args_map, long *args, void **v_args);

  // Maps FChmodat System Calls <field, value> pairs
  void makeFChmodatArgsMap(void **args_map, long *args, void **v_args);

  // Maps Link System Call <field, value> pairs
  void makeLinkArgsMap(void **args_map, long *args, void **v_args);

  // Maps Linkat System Call <field, value> pairs
  void makeLinkatArgsMap(void **args_map, long *args, void **v_args);

  // Maps Symlink System Call <field, value> pairs
  void makeSymlinkArgsMap(void **args_map, long *args, void **v_args);

  // Maps Symlinkat System Call <field, value> pairs
  void makeSymlinkatArgsMap(void **args_map, long *args, void **v_args);

  // Maps Truncate System Call <field, value> pairs
  void makeTruncateArgsMap(void **args_map, long *args, void **v_args);

  // Maps FTruncate System Call <field, value> pairs
  void makeFTruncateArgsMap(void **args_map, long *args, void **v_args);

  // Maps Access System Call <field, value> pairs
  void makeAccessArgsMap(void **args_map, long *args, void **v_args);

  // Maps FAccessat System Call <field, value> pairs
  void makeFAccessatArgsMap(void **args_map, long *args, void **v_args);

  /*
   * Maps individual mount option flags for faccessat system call to its
   * corresponding field name
   */
  u_int processFAccessatFlags(void **args_map, u_int faccessat_flags);

  // Maps individual Access mode bits to the corresponding field names
  mode_t processAccessMode(void **args_map, long *args, u_int mode_offset);

  // Maps LSeek System Call <field, value> pairs
  void makeLSeekArgsMap(void **args_map, long *args, void **v_args);

  // Maps PRead System Call <field, value> pairs
  void makePReadArgsMap(void **args_map, long *args, void **v_args);

  // Maps PWrite System Call <field, value> pairs
  void makePWriteArgsMap(void **args_map, long *args, void **v_args);

  // Maps Setpgid System Call <field, value> pairs
  void makeSetpgidArgsMap(void **args_map, long *args, void **v_args);

  // Maps Setsid System Call <field, value> pairs
  void makeSetsidArgsMap(void **args_map, long *args, void **v_args);

  // Maps Stat System Call <field, value> pairs
  void makeStatArgsMap(void **args_map, long *args, void **v_args);

  // Maps Statfs System Call <field, value> pairs
  void makeStatfsArgsMap(void **args_map, long *args, void **v_args);

  /*
   * Maps individual mount option flags for statfs system call to its
   * corresponding field name
   */
  u_int processStatfsFlags(void **args_map, u_int statfs_flags);

  // Maps FStatfs System Call <field, value> pairs
  void makeFStatfsArgsMap(void **args_map, long *args, void **v_args);

  // Maps Chown System Call <field, value> pairs
  void makeChownArgsMap(void **args_map, long *args, void **v_args);

  // Maps Readlink System Call <field, value> pairs
  void makeReadlinkArgsMap(void **args_map, long *args, void **v_args);

  // Maps Readv System Call <field, value> pairs
  void makeReadvArgsMap(void **args_map, long *args, void **v_args);

  // Maps Writev System Call <field, value> pairs
  void makeWritevArgsMap(void **args_map, long *args, void **v_args);

  // Maps Utime System Call <field, value> pairs
  void makeUtimeArgsMap(void **args_map, long *args, void **v_args);

  // Maps LStat System Call <field, value> pairs
  void makeLStatArgsMap(void **args_map, long *args, void **v_args);

  // Maps FStat System Call <field, value> pairs
  void makeFStatArgsMap(void **args_map, long *args, void **v_args);

  /*
   * Maps individual mount option flags for fstatat system call to its
   * corresponding field name
   */
  u_int processFStatatFlags(void **args_map, u_int fstatat_flags);

  // Maps FStatat System Call <field, value> pairs
  void makeFStatatArgsMap(void **args_map, long *args, void **v_args);

  // Maps Utimes System Call <field, value> pairs
  void makeUtimesArgsMap(void **args_map, long *args, void **v_args);

  // Maps Utimensat System Call <field, value> pairs
  void makeUtimensatArgsMap(void **args_map, long *args, void **v_args);

  // Maps Rename System Call <field, value> pairs
  void makeRenameArgsMap(void **args_map, long *args, void **v_args);

  // Maps Fsync System Call <field, value> pairs
  void makeFsyncArgsMap(void **args_map, long *args, void **v_args);

  // Maps FDatasync System Call <field, value> pairs
  void makeFdatasyncArgsMap(void **args_map, long *args, void **v_args);

  // Maps Fallocate System Call <field, value> pairs
  void makeFallocateArgsMap(void **args_map, long *args, void **v_args);

  // Maps Readahead System Call <field, value> pairs
  void makeReadaheadArgsMap(void **args_map, long *args, void **v_args);

  // Maps Mknod System Call <field, value> pairs
  void makeMknodArgsMap(void **args_map, long *args, void **v_args);

  // Maps Mknodat System Call <field, value> pairs
  void makeMknodatArgsMap(void **args_map, long *args, void **v_args);

  // Processes, encodes, and maps the type field for the Mknod system call
  mode_t processMknodType(void **args_map, mode_t mode);

  // Maps Pipe System Call <field, value> pairs
  void makePipeArgsMap(void **args_map, long *args, void **v_args);

  // Maps Dup System Call <field, value> pairs
  void makeDupArgsMap(void **args_map, long *args, void **v_args);

  // Maps Dup2 System Call <field, value> pairs
  void makeDup2ArgsMap(void **args_map, long *args, void **v_args);

  // Maps Dup3 System Call <field, value> pairs
  void makeDup3ArgsMap(void **args_map, long *args, void **v_args);

  // Maps Fcntl System Call <field, value> pairs
  void makeFcntlArgsMap(void **args_map, long *args, void **v_args);

  /*
   * Maps the status flag value passed to an Fcntl system call with the
   * F_SETFL command to the corresponding map fields
   */
  u_int processFcntlStatusFlags(void **args_map, u_int status_flag);

  /*
   * Maps the values in an flock structure passed to an Fcntl system call
   * with a F_SETLK, F_SETLKW, or F_GETLK command to the corresponding
   * map fields
   */
  void processFcntlFlock(void **args_map, struct flock *lock);

  /*
   * Processes the type value in an flock structure passed to an Fcntl
   * system call and sets the corresponding map field to True
   */
  void processFcntlFlockType(void **args_map, struct flock *lock);

  /*
   * Processes the whence value in an flock structure passed to an Fcntl
   * system call and sets the corresponding map field to True
   */
  void processFcntlFlockWhence(void **args_map, struct flock *lock);

  /*
   * Processes the lease argument passed to an Fcntl system call with a
   * F_SETLEASE or F_GETLEASE command and sets the corresponding map
   * field to True
   */
  void processFcntlLease(void **args_map, int lease);

  /*
   * Processes the notify argument passed to an Fcntl system call with
   * a F_NOTIFY command to the corresponding map fields
   */
  u_int processFcntlNotify(void **args_map, long *args);

  // Maps Exit System Call <field, value> pairs
  void makeExitArgsMap(void **args_map, long *args, void **v_args);

  // Maps Execve System Call <field, value> pairs
  void makeExecveArgsMap(void **args_map, long *args, void **v_args);

  // Maps Mmap System Call <field, value> pairs
  void makeMmapArgsMap(void **args_map, long *args, void **v_args);

  /*
   * Maps individual protection bits for Mmap system call to its corresponding
   * field name.
   */
  u_int processMmapProtectionArgs(void **args_map, u_int mmap_prot_flags);

  /*
   * Maps individual flag value for Mmap system call to its corresponding
   * field name.
   */
  u_int processMmapFlags(void **args_map, u_int flag);

  // Maps Munmap System Call <field, value> pairs
  void makeMunmapArgsMap(void **args_map, long *args, void **v_args);

  // Maps Getdents System Call <field, value> pairs
  void makeGetdentsArgsMap(void **args_map, long *args, void **v_args);

  // Maps Getrlimit System Call <field, value> pairs
  void makeGetrlimitArgsMap(void **args_map, long *args, void **v_args);

  // Maps Setrlimit System Call <field, value> pairs
  void makeSetrlimitArgsMap(void **args_map, long *args, void **v_args);

  // Maps Ioctl System Call <field, value> pairs
  void makeIoctlArgsMap(void **args_map, long *args, void **v_args);

  // Maps Clone System Call <field, value> pairs
  void makeCloneArgsMap(void **args_map, long *args, void **v_args);

  /*
   * Maps individual flag value for Clone system call to its corresponding
   * field name.
   */
  u_int processCloneFlags(void **args_map, u_int flag);

  /*
   * Maps individual signal value for Clone system call to its corresponding
   * field name.
   */
  u_int processCloneSignals(void **args_map, u_int flag);

  // Maps VFork System Call <field, value> pairs
  void makeVForkArgsMap(void **args_map, long *args, void **v_args);

  // Maps epoll_create System Call <field, value> pairs
  void makeEpollCreateArgsMap(void **args_map, long *args, void **v_args);

  // Maps epoll_create1 System Call <field, value> pairs
  void makeEpollCreate1ArgsMap(void **args_map, long *args, void **v_args);

  // Maps Socket System Call <field, value> pairs
  void makeSocketArgsMap(void **args_map, long *args, void **v_args);

  // Maps Connect System Call <field, value> pairs
  void makeConnectArgsMap(void **args_map, long *args, void **v_args);

  // Maps Bind System Call <field, value> pairs
  void makeBindArgsMap(void **args_map, long *args, void **v_args);

  // Maps Accept System Call <field, value> pairs
  void makeAcceptArgsMap(void **args_map, long *args, void **v_args);

  // Maps Accept4 System Call <field, value> pairs
  void makeAccept4ArgsMap(void **args_map, long *args, void **v_args);

  // Maps Getsockname System Call <field, value> pairs
  void makeGetsocknameArgsMap(void **args_map, long *args, void **v_args);

  // Maps Getpeername System Call <field, value> pairs
  void makeGetpeernameArgsMap(void **args_map, long *args, void **v_args);

  // Maps Listen System Call <field, value> pairs
  void makeListenArgsMap(void **args_map, long *args, void **v_args);

  // Maps Shutdown System Call <field, value> pairs
  void makeShutdownArgsMap(void **args_map, long *args, void **v_args);

  // Maps Getsockopt System Call <field, value> pairs
  void makeGetsockoptArgsMap(void **args_map, long *args, void **v_args);

  // Maps Setsockopt System Call <field, value> pairs
  void makeSetsockoptArgsMap(void **args_map, long *args, void **v_args);

  // Maps Socketpair System Call <field, value> pairs
  void makeSocketpairArgsMap(void **args_map, long *args, void **v_args);

  // Maps Recv System Call <field, value> pairs
  void makeRecvArgsMap(void **args_map, long *args, void **v_args);
  /*
   * Maps individual flag value for recv system call to its corresponding
   * field name.
   */
  u_int processRecvFlags(void **args_map, u_int recv_flags);

  // Maps Recvfrom System Call <field, value> pairs
  void makeRecvfromArgsMap(void **args_map, long *args, void **v_args);

  // Maps Recvmsg System Call <field, value> pairs
  void makeRecvmsgArgsMap(void **args_map, long *args, void **v_args);

  // Maps Send System Call <field, value> pairs
  void makeSendArgsMap(void **args_map, long *args, void **v_args);
  /*
   * Maps individual flag value for Send system call to its corresponding
   * field name.
   */
  u_int processSendFlags(void **args_map, u_int send_flags);

  // Maps Sendto System Call <field, value> pairs
  void makeSendtoArgsMap(void **args_map, long *args, void **v_args);

  // Maps Sendmsg System Call <field, value> pairs
  void makeSendmsgArgsMap(void **args_map, long *args, void **v_args);
};

#endif  // USE_ENUMS
#endif  // DATA_SERIES_OUTPUT_MODULE_HPP
