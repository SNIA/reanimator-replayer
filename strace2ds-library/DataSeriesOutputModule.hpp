/*
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * FIX ME: fix following comments
 * 2ds.
 * This program is similar to DataSeries's csv2ds utility, but handles
 * extents with different types and nullable fields and is primarily used
 * for converting system call csv traces.
 *
 * Usage: ./csv2ds <outputfile> <tablefile> <spec_string_file>
 *        <xml directory> <inputfiles...>
 *
 * <outputfile>: name of the dataseries output file
 * <tablefile>: name of the table file to refer to
 * <spec_string_file>: name of the file that contains a string
 *                    that specifies the format of the input file
 * <xml directory>: directory path that contains extent xml.
 *                 Remember to '/' should be the last character.
 * <inputfiles...>: input CSV files
 */

#ifndef DATA_SERIES_OUTPUT_MODULE_HPP
#define DATA_SERIES_OUTPUT_MODULE_HPP

#include <cassert>

#include <iostream>
#include <utility>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <DataSeries/ExtentType.hpp>
#include <DataSeries/DataSeriesFile.hpp>
#include <DataSeries/DataSeriesModule.hpp>

#include <strace2ds.h>

#include <fcntl.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* map<fieldname, pair<nullable, ExtentType> */
typedef std::map<std::string,
		 std::pair<bool, ExtentType::fieldType>
		 > config_table_entry_type;

/* map<extentname, config_table_entry_type> */
typedef std::map<std::string,config_table_entry_type > config_table_type;

/* map<fieldName, <DS Field, DS field type>*/
typedef std::map<std::string,
		 std::pair<void *, ExtentType::fieldType> > FieldMap;

/* map<syscallName, FieldMap> */
typedef std::map<std::string, FieldMap> ExtentMap;

// map<extent name, OutputModule>
typedef std::map<std::string, OutputModule*> OutputModuleMap;

// map<extent name, void *>
typedef std::map<std::string, void *> SysCallArgsMap;

class DataSeriesOutputModule {
public:
  // Constructor to set up all extents and fields
  DataSeriesOutputModule(std::ifstream &table_stream,
			 const std::string xml_dir,
			 const char *output_file);

  // Register the record and field values into DS fields
  bool writeRecord(const char *extent_name, long *args,
		   void *common_fields[DS_NUM_COMMON_FIELDS], void **v_args);

  // Destructor to delete the module
  ~DataSeriesOutputModule();

private:
  OutputModuleMap modules_;
  ExtentMap extents_;
  /* Sink is a wrapper for a DataSeries output file. */
  DataSeriesSink ds_sink_;
  config_table_type config_table_;
  u_int record_num_;

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
  void setField(const std::string &extent_name,
		const std::string &field_name,
		void *field_value,
		u_int var32);

  // Set corresponding DS field to null
  void setFieldNull(const std::string &extent_name,
		    const std::string &field_name);

  template <typename FieldType, typename ValueType>
  void doSetField(const std::string &extent_name,
		  const std::string &field_name,
		  void *field_value);

  // Returns the length for field of type variable32
  u_int getVariable32FieldLength(SysCallArgsMap &args_map,
				 const std::string &field_name);

  // Initialize args map for given system call
  void initArgsMap(SysCallArgsMap &args_map, const char *extent_name);

  // Maps Close System Call <field, value> pairs
  void makeCloseArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Open System Call <field, value> pairs
  void makeOpenArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Openat System Call <field, value> pairs
  void makeOpenatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Processes individual flag and mode bits
  void process_Flag_and_Mode_Args(SysCallArgsMap &args_map,
				  unsigned int &num,
				  int value,
				  std::string field_name);

  /*
   * Maps individual flag value for Open system call to its corresponding
   * field name.
   */
  u_int processOpenFlags(SysCallArgsMap &args_map, u_int flag);

  // Maps individual mode bits of mode argument to its corresponding field name
  mode_t processMode(SysCallArgsMap &args_map, long *args, u_int offset);

  // Convert time from a timeval to a uint64_t in Tfracs
  uint64_t timeval_to_Tfrac(struct timeval tv);

  // Convert time from a long (seconds) to a uint64_t in Tfracs
  uint64_t sec_to_Tfrac(time_t time);

  // Convert time from a timespec to a uint64_t in Tfracs
  uint64_t timespec_to_Tfrac(struct timespec ts);

  // Maps Read System Call <field, value> pairs
  void makeReadArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

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

  // Maps Creat System Call <field, value> pairs
  void makeCreatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Chmod System Calls <field, value> pairs
  void makeChmodArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Link System Call <field, value> pairs
  void makeLinkArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Symlink System Call <field, value> pairs
  void makeSymlinkArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Truncate System Call <field, value> pairs
  void makeTruncateArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Access System Call <field, value> pairs
  void makeAccessArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps individual Access mode bits to the corresponding field names
  mode_t processAccessMode(SysCallArgsMap &args_map,
			   long *args,
			   u_int mode_offset);

  // Maps LSeek System Call <field, value> pairs
  void makeLSeekArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps PRead System Call <field, value> pairs
  void makePReadArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps PWrite System Call <field, value> pairs
  void makePWriteArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Stat System Call <field, value> pairs
  void makeStatArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

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

  // Maps Utimes System Call <field, value> pairs
  void makeUtimesArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Rename System Call <field, value> pairs
  void makeRenameArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Fsync System Call <field, value> pairs
  void makeFsyncArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Mknod System Call <field, value> pairs
  void makeMknodArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

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

  // Maps Execve System Call <field, value> pairs
  void makeExecveArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  // Maps Mmap System Call <field, value> pairs
  void makeMmapArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);

  /*
   * Maps individual protection bits for Mmap system call to its corresponding
   * field name.
   */
  u_int processMmapProtectionArgs(SysCallArgsMap &args_map, u_int mmap_prot_flags);

  /*
   * Maps individual flag value for Mmap system call to its corresponding
   * field name.
   */
  u_int processMmapFlags(SysCallArgsMap &args_map, u_int flag);

  // Maps Getdents System Call <field, value> pairs
  void makeGetdentsArgsMap(SysCallArgsMap &args_map, long *args, void **v_args);
};

#endif // DATA_SERIES_OUTPUT_MODULE_HPP
