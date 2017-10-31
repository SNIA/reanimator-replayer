/*
 * Copyright (c) 2016-2016 Nina Brown
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
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

#include <cassert>

#include <iostream>
#include <utility>
#include <unordered_map>
#include <sstream>

#include <DataSeries/ExtentType.hpp>
#include <DataSeries/DataSeriesFile.hpp>
#include <DataSeries/DataSeriesModule.hpp>

#include <strace2ds.h>

#include <fcntl.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sched.h>
#include <signal.h>
#include <sys/xattr.h>
#include <sys/time.h>
#include <sys/resource.h>
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

/* map<fieldname, pair<nullable, ExtentType> */
typedef std::unordered_map<int,
		 std::pair<bool, ExtentType::fieldType>
		 > config_table_entry_type;

/* map<extentname, config_table_entry_type> */
typedef std::unordered_map<std::string,
	config_table_entry_type> config_table_type;

/* pair<DS Field, DS field type> */
typedef std::pair<void *, ExtentType::fieldType> ExtentFieldTypePair;

/* map<fieldName, <DS Field, DS field type>*/
typedef std::unordered_map<std::string, ExtentFieldTypePair> FieldMap;

/* map<syscallName, FieldMap> */
typedef std::unordered_map<std::string, FieldMap> ExtentMap;

// map<extent name, OutputModule>
typedef std::unordered_map<std::string, OutputModule*> OutputModuleMap;

// map<extent name, void *>
typedef std::unordered_map<std::string, void *> SysCallArgsMap;

// function pointer type for system call args map
typedef void (DataSeriesOutputModule::*SysCallArgsMapFuncPtr)(SysCallArgsMap &,
							      long *,
							      void **);
// map<extent_name, SysCallArgsMapFuncPtr>
typedef std::unordered_map<std::string, SysCallArgsMapFuncPtr> FuncPtrMap;

// function pointer type for system call args map
typedef void (DataSeriesOutputModule::*SysCallArgsMapFuncPtr2)(void **,
							      long *,
							      void **);
// map<extent_name, SysCallArgsMapFuncPtr>
typedef std::unordered_map<std::string, SysCallArgsMapFuncPtr2> FuncPtrMap2;


extern unsigned nsyscalls;

class DataSeriesOutputModule {
public:
  static bool true_;
  static bool false_;
  //#define USE_ENUMS 1
 // void *SysCallArgsArray[MAX_SYSCALL_FIELDS];
  std::string FieldNames[MAX_SYSCALL_FIELDS];

  // A map of untraced syscalls number and their respective counts
  std::unordered_map<long, int> untraced_sys_call_counts_;

  // Constructor to set up all extents and fields
  DataSeriesOutputModule(std::ifstream &table_stream,
			 const std::string xml_dir,
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
  int64_t getNextID();

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
   * Since ExtentMap's value is of type FieldMap, we create a cache
   * of type FieldMap
   */
  FieldMap **extents_cache_;

  /* Sink is a wrapper for a DataSeries output file. */
  DataSeriesSink ds_sink_;
  config_table_type config_table_;
  /*
   * Since config_table_type's value is of type config_table_entry_type,
   * we create a cache of type config_table_entry_type
   */
  config_table_entry_type **config_table_cache_;

  int64_t record_num_;
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
  FuncPtrMap2 func_ptr_map_2;
  /*
   * Since FuncPtrMap's value is of type SysCallArgsMapFuncPtr, we create
   * a cache of type SysCallArgsMapFuncPtr
   */
  SysCallArgsMapFuncPtr *func_ptr_map_cache_;
  SysCallArgsMapFuncPtr2 *func_ptr_map_cache_2;

  // Disable copy constructor
  DataSeriesOutputModule(const DataSeriesOutputModule&);

  // Initialize config table
  void initConfigTable(std::ifstream &table_stream);

  // Add a extent(system call)
  void addExtent(const std::string &extent_name, ExtentSeries &series);

  // Add a field(system call arg) to the desired extent
  void addField(const std::string &extent_name,
		const std::string &field_name,
		void *field,
		const ExtentType::fieldType field_type);

  // Set corresponding DS field to the given value
  void setField(const ExtentFieldTypePair& extent_field_value_,
		void *field_value,
		int var32);

  // Set corresponding DS field to null
  void setFieldNull(const ExtentFieldTypePair& extent_field_value_);

  template <typename FieldType, typename ValueType>
  void doSetField(const ExtentFieldTypePair& extent_field_value_,
		  void *field_value);

  /*
   * Creates mapping of sys call name with the address
   * of it corresponding args map function
   */
  void initArgsMapFuncPtr();

  // Initializes all the caches with NULL values
  void initCache();

  // Returns the length for field of type variable32
  int getVariable32FieldLength(void **args_map,
			       const int &field_enum);

  // Initialize args map for given system call
  void initArgsMap(SysCallArgsMap &args_map, const char *extent_name);

  // Initialize args map for given system call
  void initArgsMap2(void **args_map, const char *extent_name);

  // Maps Close System Call <field, value> pairs
  void makeCloseArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Close System Call <field, value> pairs
  void makeCloseArgsMap2(void **args_map, long *args, void **v_args);

  // Maps Open System Call <field, value> pairs
  void makeOpenArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Open System Call <field, value> pairs
  void makeOpenArgsMap2(void **args_map, long *args, void **v_args);

  // Maps Openat System Call <field, value> pairs
  void makeOpenatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Processes individual flag and mode bits
  void process_Flag_and_Mode_Args(SysCallArgsMap &args_map,
				  unsigned int &num,
				  int value,
				  std::string field_name);

  // Processes individual flag and mode bits
  void process_Flag_and_Mode_Args2(void **args_map,
				  unsigned int &num,
				  int value,
				  int field_enum);

  /*
   * Maps individual flag value for Open system call to its corresponding
   * field name.
   */
  u_int processOpenFlags(SysCallArgsMap &args_map, u_int flag);

  /*
   * Maps individual flag value for Open system call to its corresponding
   * field name.
   */
  u_int processOpenFlags2(void **args_map, u_int flag);

  // Maps individual mode bits of mode argument to its corresponding field name
  mode_t processMode(SysCallArgsMap &args_map, long *args, u_int offset);

  // Maps individual mode bits of mode argument to its corresponding field name
  mode_t processMode2(void **args_map, long *args, u_int offset);

  // Convert time from a timeval to a uint64_t in Tfracs
  uint64_t timeval_to_Tfrac(struct timeval tv);

  // Convert time from a long (seconds) to a uint64_t in Tfracs
  uint64_t sec_to_Tfrac(time_t time);

  // Convert time from a timespec to a uint64_t in Tfracs
  uint64_t timespec_to_Tfrac(struct timespec ts);

  // Maps Read System Call <field, value> pairs
  void makeReadArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Read System Call <field, value> pairs
  void makeReadArgsMap2(void **args_map, long *args, void **v_args);

  // Maps Write System Call <field, value> pairs
  void makeWriteArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Chdir System Call <field, value> pairs
  void makeChdirArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Rmdir System Call <field, value> pairs
  void makeRmdirArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Unlink System Call <field, value> pairs
  void makeUnlinkArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Unlinkat System Call <field, value> pairs
  void makeUnlinkatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Mkdir System Call <field, value> pairs
  void makeMkdirArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Mkdirat System Call <field, value> pairs
  void makeMkdiratArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Creat System Call <field, value> pairs
  void makeCreatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Chmod System Calls <field, value> pairs
  void makeChmodArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Umask System Calls <field, value> pairs
  void makeUmaskArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Umask System Calls <field, value> pairs
  void makeUmaskArgsMap2(void **args_map, long *args, void **v_args);

  // Maps Setxattr System Calls <field, value> pairs
  void makeSetxattrArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps LSetxattr System Calls <field, value> pairs
  void makeLSetxattrArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Getxattr System Calls <field, value> pairs
  void makeGetxattrArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps LGetxattr System Calls <field, value> pairs
  void makeLGetxattrArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps FSetxattr System Calls <field, value> pairs
  void makeFSetxattrArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps FGetxattr System Calls <field, value> pairs
  void makeFGetxattrArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Listxattr System Calls <field, value> pairs
  void makeListxattrArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps LListxattr System Calls <field, value> pairs
  void makeLListxattrArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps FListxattr System Calls <field, value> pairs
  void makeFListxattrArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps FLock System Calls <field, value> pairs
  void makeFLockArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Removexattr System Calls <field, value> pairs
  void makeRemovexattrArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps LRemovexattr System Calls <field, value> pairs
  void makeLRemovexattrArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps FRemovexattr System Calls <field, value> pairs
  void makeFRemovexattrArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps FChmod System Calls <field, value> pairs
  void makeFChmodArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps FChmodat System Calls <field, value> pairs
  void makeFChmodatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Link System Call <field, value> pairs
  void makeLinkArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Linkat System Call <field, value> pairs
  void makeLinkatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Symlink System Call <field, value> pairs
  void makeSymlinkArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Symlinkat System Call <field, value> pairs
  void makeSymlinkatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Truncate System Call <field, value> pairs
  void makeTruncateArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps FTruncate System Call <field, value> pairs
  void makeFTruncateArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Access System Call <field, value> pairs
  void makeAccessArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Access System Call <field, value> pairs
  void makeAccessArgsMap2(void **args_map, long *args, void **v_args);

  // Maps FAccessat System Call <field, value> pairs
  void makeFAccessatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  /*
   * Maps individual mount option flags for faccessat system call to its
   * corresponding field name
   */
  u_int processFAccessatFlags(SysCallArgsMap &args_map, u_int faccessat_flags);

  // Maps individual Access mode bits to the corresponding field names
  mode_t processAccessMode(SysCallArgsMap &args_map,
			   long *args,
			   u_int mode_offset);

  // Maps individual Access mode bits to the corresponding field names
  mode_t processAccessMode2(void **args_map,
			   long *args,
			   u_int mode_offset);

  // Maps LSeek System Call <field, value> pairs
  void makeLSeekArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps LSeek System Call <field, value> pairs
  void makeLSeekArgsMap2(void **args_map, long *args, void **v_args);

  // Maps PRead System Call <field, value> pairs
  void makePReadArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps PWrite System Call <field, value> pairs
  void makePWriteArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Setpgid System Call <field, value> pairs
  void makeSetpgidArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Setsid System Call <field, value> pairs
  void makeSetsidArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Stat System Call <field, value> pairs
  void makeStatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Statfs System Call <field, value> pairs
  void makeStatfsArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  /*
   * Maps individual mount option flags for statfs system call to its
   * corresponding field name
   */
  u_int processStatfsFlags(SysCallArgsMap &args_map, u_int statfs_flags);

  // Maps FStatfs System Call <field, value> pairs
  void makeFStatfsArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Chown System Call <field, value> pairs
  void makeChownArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Readlink System Call <field, value> pairs
  void makeReadlinkArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Readv System Call <field, value> pairs
  void makeReadvArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Writev System Call <field, value> pairs
  void makeWritevArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Utime System Call <field, value> pairs
  void makeUtimeArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps LStat System Call <field, value> pairs
  void makeLStatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps FStat System Call <field, value> pairs
  void makeFStatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps FStat System Call <field, value> pairs
  void makeFStatArgsMap2(void **args_map, long *args, void **v_args);

  /*
   * Maps individual mount option flags for fstatat system call to its
   * corresponding field name
   */
  u_int processFStatatFlags(SysCallArgsMap &args_map, u_int fstatat_flags);

  // Maps FStatat System Call <field, value> pairs
  void makeFStatatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Utimes System Call <field, value> pairs
  void makeUtimesArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Utimensat System Call <field, value> pairs
  void makeUtimensatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Rename System Call <field, value> pairs
  void makeRenameArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Fsync System Call <field, value> pairs
  void makeFsyncArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Mknod System Call <field, value> pairs
  void makeMknodArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Mknodat System Call <field, value> pairs
  void makeMknodatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Processes, encodes, and maps the type field for the Mknod system call
  mode_t processMknodType(SysCallArgsMap &args_map, mode_t mode);

  // Maps Pipe System Call <field, value> pairs
  void makePipeArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Dup System Call <field, value> pairs
  void makeDupArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Dup2 System Call <field, value> pairs
  void makeDup2ArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Fcntl System Call <field, value> pairs
  void makeFcntlArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  /*
   * Maps the status flag value passed to an Fcntl system call with the
   * F_SETFL command to the corresponding map fields
   */
  u_int processFcntlStatusFlags(SysCallArgsMap &args_map, u_int status_flag);

  /*
   * Maps the values in an flock structure passed to an Fcntl system call
   * with a F_SETLK, F_SETLKW, or F_GETLK command to the corresponding
   * map fields
   */
  void processFcntlFlock(SysCallArgsMap &args_map, struct flock *lock);

  /*
   * Processes the type value in an flock structure passed to an Fcntl
   * system call and sets the corresponding map field to True
   */
  void processFcntlFlockType(SysCallArgsMap &args_map, struct flock *lock);

  /*
   * Processes the whence value in an flock structure passed to an Fcntl
   * system call and sets the corresponding map field to True
   */
  void processFcntlFlockWhence(SysCallArgsMap &args_map, struct flock *lock);

  /*
   * Processes the lease argument passed to an Fcntl system call with a
   * F_SETLEASE or F_GETLEASE command and sets the corresponding map
   * field to True
   */
  void processFcntlLease(SysCallArgsMap &args_map, int lease);

  /*
   * Processes the notify argument passed to an Fcntl system call with
   * a F_NOTIFY command to the corresponding map fields
   */
  u_int processFcntlNotify(SysCallArgsMap &args_map, long *args);

  // Maps Exit System Call <field, value> pairs
  void makeExitArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Exit System Call <field, value> pairs
  void makeExitArgsMap2(void **args_map, long *args, void **v_args);

  // Maps Execve System Call <field, value> pairs
  void makeExecveArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Execve System Call <field, value> pairs
  void makeExecveArgsMap2(void **args_map, long *args, void **v_args);

  // Maps Mmap System Call <field, value> pairs
  void makeMmapArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Mmap System Call <field, value> pairs
  void makeMmapArgsMap2(void **args_map, long *args, void **v_args);

  /*
   * Maps individual protection bits for Mmap system call to its corresponding
   * field name.
   */
  u_int processMmapProtectionArgs(SysCallArgsMap &args_map, u_int mmap_prot_flags);

  /*
   * Maps individual protection bits for Mmap system call to its corresponding
   * field name.
   */
  u_int processMmapProtectionArgs2(void **args_map, u_int mmap_prot_flags);

  /*
   * Maps individual flag value for Mmap system call to its corresponding
   * field name.
   */
  u_int processMmapFlags(SysCallArgsMap &args_map, u_int flag);

  /*
   * Maps individual flag value for Mmap system call to its corresponding
   * field name.
   */
  u_int processMmapFlags2(void **args_map, u_int flag);

  // Maps Munmap System Call <field, value> pairs
  void makeMunmapArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Munmap System Call <field, value> pairs
  void makeMunmapArgsMap2(void **args_map, long *args, void **v_args);

  // Maps Getdents System Call <field, value> pairs
  void makeGetdentsArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Getrlimit System Call <field, value> pairs
  void makeGetrlimitArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Setrlimit System Call <field, value> pairs
  void makeSetrlimitArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Ioctl System Call <field, value> pairs
  void makeIoctlArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Clone System Call <field, value> pairs
  void makeCloneArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  /*
   * Maps individual flag value for Clone system call to its corresponding
   * field name.
   */
  u_int processCloneFlags(SysCallArgsMap &args_map, u_int flag);

  /*
   * Maps individual signal value for Clone system call to its corresponding
   * field name.
   */
  u_int processCloneSignals(SysCallArgsMap &args_map, u_int flag);

  // Maps VFork System Call <field, value> pairs
  void makeVForkArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Socket System Call <field, value> pairs
  void makeSocketArgsMap(SysCallArgsMap &args_map,
		  	 long *args, void **v_args);
};
#endif // DATA_SERIES_OUTPUT_MODULE_HPP
