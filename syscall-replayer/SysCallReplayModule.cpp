/*
 * Copyright (c) 2015 Leixiang Wu
 * Copyright (c) 2011 Jack Ma
 * Copyright (c) 2011 Vasily Tarasov
 * Copyright (c) 2011 Santhosh Kumar Koundinya
 * Copyright (c) 2011 Erez Zadok
 * Copyright (c) 2011 Geoff Kuenning
 * Copyright (c) 2011 Stony Brook University
 * Copyright (c) 2011 Harvey Mudd College
 * Copyright (c) 2011 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <iostream>
#include <string>
#include <sstream>

#include <algorithm>
#include <vector>
#include <queue>         
#include <set>

#include <DataSeries/PrefetchBufferModule.hpp>
#include <DataSeries/TypeIndexModule.hpp>
#include <DataSeries/RowAnalysisModule.hpp>

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <cstring>
#include <csignal>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <fcntl.h>

#include <stdexcept>

#define DEFAULT_MODE 0
#define WARN_MODE    1
#define ABORT_MODE   2

class SystemCallTraceReplayModule : public RowAnalysisModule {
protected:
  std::string sys_call_;
  bool verbose_;
  int  warn_level_;
  DoubleField time_called_;
  Int64Field return_value_;
  bool completed;
  static std::map<int, int> fd_map;
public:
  SystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag, int warn_level_flag) : 
    RowAnalysisModule(source),
    verbose_(verbose_flag),
    warn_level_(warn_level_flag),
    time_called_(series, "time_called"),
    return_value_(series, "return_value", Field::flag_nullable),
    completed(false) {
  }

  double time_called() {
    return (double)time_called_.val();
  }
  
  Extent::Ptr getSharedExtent() {
    Extent::Ptr e = source.getSharedExtent();
    if (e == NULL) {
      if (!completed) {
	completed = true;
	completeProcessing();
      }
      return e;
    }
    completed = false;
    if (!prepared) {
      firstExtent(*e);
    }
    newExtentHook(*e);
    series.setExtent(e);
    if (!prepared) {
      prepareForProcessing();
      prepared = true;
    }
    return e;
  }
  bool has_more_trace() {
    if (series.morerecords()) {
      return true;
    } else {
      series.clearExtent();
      return false;
    }
  }
  void execute() {
    ++processed_rows;
    processRow();
    ++series;
  }

  void compare_retval(int ret_val) {
    if (verbose_){
      std::cout << "Captured return value: " << return_value_.val() << ", ";
      std::cout << "Replayed return value: " << ret_val << std::endl;
    }

    if (warn_level_ == WARN_MODE && return_value_.val() != ret_val) {
      std::cout << "time called:" << std::fixed << time_called() << std::endl;
      std::cout << "Captured return value is different from replayed return value" << std::endl;
      std::cout << "Captured return value: " << return_value_.val() << ", ";
      std::cout << "Replayed return value: " << ret_val << std::endl;
    }else if (warn_level_ == ABORT_MODE && return_value_.val() != ret_val) {
      abort();
    }
  }
};

struct LessThanByTimeCalled {
  bool operator()(SystemCallTraceReplayModule* m1, SystemCallTraceReplayModule* m2) const {
    // min heap uses this to sort the tree.
    if (m1->time_called() >= m2->time_called())
      return true;
    else 
      return false;
  }
};


class OpenSystemCallTraceReplayModule : public SystemCallTraceReplayModule {

private:
  /* DataSeries Open System Call Trace Fields */
  Variable32Field given_pathname;
  BoolField flag_read_only;
  BoolField flag_write_only;
  BoolField flag_read_and_write;
  BoolField flag_append;
  BoolField flag_async;
  BoolField flag_close_on_exec;
  BoolField flag_create;
  BoolField flag_direct;
  BoolField flag_directory;
  BoolField flag_exclusive;
  BoolField flag_largefile;
  BoolField flag_no_access_time;
  BoolField flag_no_controlling_terminal;
  BoolField flag_no_follow;
  BoolField flag_no_blocking_mode;
  BoolField flag_no_delay;
  BoolField flag_synchronous;
  BoolField flag_truncate;
  BoolField mode_R_user;
  BoolField mode_W_user;
  BoolField mode_X_user;
  BoolField mode_R_group;
  BoolField mode_W_group;
  BoolField mode_X_group;
  BoolField mode_R_others;
  BoolField mode_W_others;
  BoolField mode_X_others;

  int getFlags() {
    int access_mode_sum = flag_read_only.val() + flag_write_only.val() + flag_read_and_write.val();
    if (access_mode_sum != 1) {
      // Error since it is only allowed to set one of three access modes: O_RDONLY, O_WRONLY, or O_RDWR 
      std::cerr << "Error: Multiple access modes are set (Only one access mode O_RDONLY, O_WRONLY, or O_RDWR can be set)." << std::endl;
      return -1;
    }
    int flags = O_RDONLY;
    if (flag_read_only.val() == 1) {
      flags = O_RDONLY;
    } else if (flag_write_only.val() == 1) {
      flags = O_WRONLY;
    } else if (flag_read_and_write.val() == 1) {
      flags = O_RDWR;
    }
    if (flag_append.val() == 1) {
      flags |= O_APPEND;
    }
    if (flag_async.val() == 1) {
      flags |= O_ASYNC;
    }
    if (flag_close_on_exec.val() == 1) {
      flags |= O_CLOEXEC;
    }
    if (flag_create.val() == 1) {
      flags |= O_CREAT;
    }
    if (flag_direct.val() == 1) {
      flags |= O_DIRECT;
    }
    if (flag_directory.val() == 1) {
      flags |= O_DIRECTORY;
    }
    if (flag_exclusive.val() == 1) {
      flags |= O_EXCL;
    }
    if (flag_largefile.val() == 1) {
      flags |= O_LARGEFILE;
    }
    if (flag_no_access_time.val() == 1) {
      flags |= O_NOATIME;
    }
    if (flag_no_controlling_terminal.val() == 1) {
      flags |= O_NOCTTY;
    }
    if (flag_no_follow.val() == 1) {
      flags |= O_NOFOLLOW;
    }
    if (flag_no_blocking_mode.val() == 1) {
      flags |= O_NONBLOCK;
    }
    if (flag_no_delay.val() == 1) {
      flags |= O_NDELAY;
    }
    if (flag_synchronous.val() == 1) {
      flags |= O_SYNC;
    }
    if (flag_truncate.val() == 1) {
      flags |= O_TRUNC;
    }
    return flags;
  }

  mode_t getMode() {
    mode_t mode = 0;
    if (mode_R_user.val() == 1) {
      mode |= S_IRUSR;
    }
    if (mode_W_user.val() == 1) {
      mode |= S_IWUSR;
    }
    if (mode_X_user.val() == 1) {
      mode |= S_IXUSR;
    }
    if (mode_R_group.val() == 1) {
      mode |= S_IRGRP;
    }
    if (mode_W_group.val() == 1) {
      mode |= S_IWGRP;
    }
    if (mode_X_group.val() == 1) {
      mode |= S_IXGRP;
    }
    if (mode_R_others.val() == 1) {
      mode |= S_IROTH;
    }
    if (mode_W_others.val() == 1) {
      mode |= S_IWOTH;
    }
    if (mode_X_others.val() == 1) {
      mode |= S_IXOTH;
    }	    
    /*
      std::cout << "mode_R_user: " << mode_R_user.val() << std::endl;
      std::cout << "mode_W_user: " << mode_W_user.val() << std::endl;
      std::cout << "mode_X_user: " << mode_X_user.val() << std::endl;
      std::cout << "mode_R_group: " << mode_R_group.val() << std::endl;
      std::cout << "mode_W_group: " << mode_W_group.val() << std::endl;
      std::cout << "mode_X_group: " << mode_X_group.val() << std::endl;
      std::cout << "mode_R_others: " << mode_R_others.val() << std::endl;
      std::cout << "mode_W_others: " << mode_W_others.val() << std::endl;
      std::cout << "mode_X_others: " << mode_X_others.val() << std::endl;
    */
    return mode;
  }

public:
  OpenSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag, int warn_level_flag) : 
    SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
    given_pathname(series, "given_pathname"),
    flag_read_only(series, "flag_read_only"),
    flag_write_only(series, "flag_write_only"),
    flag_read_and_write(series, "flag_read_and_write"),
    flag_append(series, "flag_append"),
    flag_async(series, "flag_async"),
    flag_close_on_exec(series, "flag_close_on_exec"),
    flag_create(series, "flag_create"),
    flag_direct(series, "flag_direct"),
    flag_directory(series, "flag_directory"),
    flag_exclusive(series, "flag_exclusive"),
    flag_largefile(series, "flag_largefile"),
    flag_no_access_time(series, "flag_no_access_time"),
    flag_no_controlling_terminal(series, "flag_no_controlling_terminal"),
    flag_no_follow(series, "flag_no_follow"),
    flag_no_blocking_mode(series, "flag_no_blocking_mode"),
    flag_no_delay(series, "flag_no_delay"),
    flag_synchronous(series, "flag_synchronous"),
    flag_truncate(series, "flag_truncate"),
    mode_R_user(series, "mode_R_user"),
    mode_W_user(series, "mode_W_user"),
    mode_X_user(series, "mode_X_user"),
    mode_R_group(series, "mode_R_group"),
    mode_W_group(series, "mode_W_group"),
    mode_X_group(series, "mode_X_group"),
    mode_R_others(series, "mode_R_others"),
    mode_W_others(series, "mode_W_others"),
    mode_X_others(series, "mode_X_others") {
    sys_call_ = "open";
  }

  /* This function will be called by RowAnalysisModule once
   * before the first operation is replayed. */
  void prepareForProcessing() {
    std::cout << "-----Open System Call Replayer starts to replay...-----" << std::endl;
  }

  /* This function will be called by RowAnalysisModule once
   * for every system call operation in the trace file 
   * being processed */
  void processRow() {
    char *pathname = (char *)given_pathname.val();
    int flags = getFlags();
    if (flags == -1) {
      std::cout << given_pathname.val() << " is NOT successfully opened." << std::endl;
      return;
    }
    mode_t mode = getMode();
    int return_value = (int)return_value_.val();

    if (verbose_) {
      std::cout << "open: ";
      std::cout.precision(25);
      std::cout << "time called(" << std::fixed << time_called() << "), ";
      std::cout << "pathname(" << pathname << "), ";
      std::cout << "flags(" << flags << "), ";
      std::cout << "mode(" << mode << ")\n";
    }
			
    int replay_ret = open(pathname, flags, mode);
    compare_retval(replay_ret);
    fd_map[return_value] = replay_ret;

    if (replay_ret == -1) {
      perror(pathname);
    } else {
      if (verbose_) {
	std::cout << given_pathname.val() << " is successfully opened..." << std::endl;
      }
    }
  }

  /* This function will be called once by RowAnalysisModule after all
   * system operations are being replayed.*/
  void completeProcessing() {
    std::cout << "-----Open System Call Replayer finished replaying...-----" << std::endl;
  }
};

class CloseSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  /* DataSeries Close System Call Trace Fields */
  Int32Field descriptor_;

public:
  CloseSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag, int warn_level_flag) : 
    SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
    descriptor_(series, "descriptor"){
    sys_call_ = "close";
  }

  /* This function will be called by RowAnalysisModule once
   * before the first operation is replayed. */
  void prepareForProcessing() {
    std::cout << "-----Close System Call Replayer starts to replay...-----" << std::endl;
  }

  /* This function will be called by RowAnalysisModule once
   * for every system call operation in the trace file 
   * being processed */
  void processRow() {
    int fd = fd_map[descriptor_.val()];
    if (verbose_) {
      std::cout << "close: ";
      std::cout.precision(25);
      std::cout << "time called(" << std::fixed << time_called() << "), ";
      std::cout << "descriptor(" << descriptor_.val() << ")\n";
    }
	
    int ret = close(fd);
    fd_map.erase(descriptor_.val());
    compare_retval(ret);

    if (ret == -1) {
      perror("close");
    } else {
      if (verbose_) {
	std::cout << "fd " << descriptor_.val() << " is successfully closed..." << std::endl;
      }
    }
  }

  /* This function will be called once by RowAnalysisModule after all
   * system operations are being replayed.*/
  void completeProcessing() {
    std::cout << "-----Close System Call Replayer finished replaying...-----" << std::endl;
  }
};

class ReadSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  /* DataSeries Read System Call Trace Fields */
  bool verify_;
  Int32Field descriptor_;
  Variable32Field data_read_;
  Int64Field bytes_requested_;
public:
  ReadSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag, bool verify_flag, int warn_level_flag) : 
    SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
    verify_(verify_flag),
    descriptor_(series, "descriptor"), 
    data_read_(series, "data_read", Field::flag_nullable), 
    bytes_requested_(series, "bytes_requested") {
    sys_call_ = "read";
  }

  /* This function will be called by RowAnalysisModule once
   * before the first operation is replayed. */
  void prepareForProcessing() {
    std::cout << "-----Read System Call Replayer starts to replay...-----" << std::endl;
  }

  /* This function will be called by RowAnalysisModule once
   * for every system call operation in the trace file 
   * being processed */
  void processRow() {
    int fd = fd_map[descriptor_.val()];
    int nbytes = bytes_requested_.val();
    if (verbose_) {
      std::cout << sys_call_ << ": ";
      std::cout.precision(23);      
      std::cout << "time called(" << std::fixed << time_called() << "), ";
      std::cout << "descriptor:" << descriptor_.val() << "), ";
      std::cout << "data read(" << data_read_.val() << "), ";
      std::cout << "size(" << descriptor_.val() << ")\n";
    }
    char buffer[bytes_requested_.val()];
    int ret = read(fd, buffer, nbytes);
    compare_retval(ret);

    if (verify_ == true) {
      if (memcmp(data_read_.val(),buffer,ret) != 0){
        // data aren't same
        std::cerr << "Verification of data in read failed.\n";
        if (warn_level_ != DEFAULT_MODE) {
          std::cout << "time called:" << std::fixed << time_called() << std::endl;
          std::cout << "Captured read data is different from replayed read data" << std::endl;
          std::cout << "Captured read data: " << data_read_.val() << ", ";
          std::cout << "Replayed read data: " << buffer << std::endl;
	  if (warn_level_ == ABORT_MODE ) {
	    abort();
	  }
	}
      } else {
        if (verbose_) {
          std::cout << "Verification of data in read success.\n";
        }
      }
    }
    
    if (ret == -1) {
      perror("read");
    } else {
      if (verbose_) {
	std::cout << "read is executed successfully!" << std::endl;
      }
    }
  }

  /* This function will be called once by RowAnalysisModule after all
   * system operations are being replayed.*/
  void completeProcessing() {
    std::cout << "-----Read System Call Replayer finished replaying...-----" << std::endl;
  }
};

class WriteSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  /* DataSeries Write System Call Trace Fields */
  bool verify_;
  std::string pattern_data_;
  Int32Field descriptor_;
  Variable32Field data_written_;
  Int64Field bytes_requested_;
  std::ifstream random_file_;
public:
  WriteSystemCallTraceReplayModule(DataSeriesModule &source,
				   bool verbose_flag,
				   bool verify_flag,
 				   int warn_level_flag,
				   std::string pattern_data) : 
    SystemCallTraceReplayModule(source, verbose_flag, warn_level_flag),
    verify_(verify_flag),
    pattern_data_(pattern_data),
    descriptor_(series, "descriptor"), 
    data_written_(series, "data_written", Field::flag_nullable), 
    bytes_requested_(series, "bytes_requested"){
    sys_call_ = "write";
  }
  
  /* This function will be called by RowAnalysisModule once
   * before the first operation is replayed. */
  void prepareForProcessing() {
    std::cout << "-----Write System Call Replayer starts to replay...-----" << std::endl;
    if (pattern_data_ == "random") {
      random_file_.open("/dev/urandom");
      if (!random_file_.is_open()) {
	std::cerr << "Unable to open file '/dev/urandom/'.\n";
	exit(EXIT_FAILURE);
      }
    }
  }

  /* This function will be called by RowAnalysisModule once
   * for every system call operation in the trace file 
   * being processed */
  void processRow() {
    size_t nbytes = (size_t)bytes_requested_.val();
    int fd = fd_map[descriptor_.val()];
    char *buffer;
    
    // check to see if write data was in DS and user didn't specify pattern
    if (data_written_.isNull() && pattern_data_.empty()) {
      // let's write zeros.
      pattern_data_ = "0x0";
    }

    if (!pattern_data_.empty()) {
      buffer = new char[nbytes];
      if (pattern_data_ == "random") {
	random_file_.read(buffer, nbytes);
      } else {
	int pattern_hex;
	std::stringstream pattern_stream;
	pattern_stream << std::hex << pattern_data_;
	pattern_stream >> pattern_hex;
	memset(buffer, pattern_hex, nbytes);
      }
    } else {
	buffer = (char *)data_written_.val();
    }

    if (verbose_) {
      std::cout << sys_call_ << ": ";
      std::cout.precision(25);
      std::cout << "time called(" << std::fixed << time_called() << "),";
      std::cout << "descriptor(" << descriptor_.val() << "),";
      std::cout << "data(" << buffer << "),";
      std::cout << "nbytes(" << nbytes << ")" << std::endl;
    }

    int ret = write(fd, buffer, nbytes);
    compare_retval(ret);
    
    if (!pattern_data_.empty()){
      delete[] buffer;
    }
    
    if (ret == -1) {
      perror("write");
    } else {
      if (verbose_) {
	std::cout << "write is successfully replayed\n";
      }
    }
  }

  /* This function will be called once by RowAnalysisModule after all
   * system operations are being replayed.*/
  void completeProcessing() {
    std::cout << "-----Write System Call Replayer finished replaying...-----" << std::endl;
    if (pattern_data_ == "random") {
      random_file_.close();
    } 
  }
};

// define the static fd_map in SystemCallTraceReplayModule
std::map<int, int> SystemCallTraceReplayModule::fd_map;

int main(int argc, char *argv[]) {
  namespace po = boost::program_options;

  int ret = EXIT_SUCCESS;
  bool verbose = false;
  bool verify = false;
  int warn_level = DEFAULT_MODE;
  std::string pattern_data = "";
  std::vector<std::string> input_files;

  // Declare a group of options that will be 
  // allowed only on command line
  po::options_description generic("Generic options");
  generic.add_options()
    ("version,V", "print version of system call replayer")
    ("help,h", "produce help message")
    ;

  // Declare a group of options that will be 
  // allowed both on command line and in
  // config file
  po::options_description config("Configuration");
  config.add_options()
    ("verbose,v", "system calls replay in verbose mode")
    ("verify", "verifies that the data being written/read is exactly what was used originally")
    ("warn,w", po::value<int>(), "system call replays in warn mode")
    ("pattern,p", po::value<std::string>(), "write repeated pattern data in write system call")
    ;

  // Hidden options, will be allowed both on command line and
  // in config file, but will not be shown to the user.
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("input-files,I", po::value<std::vector<std::string> >(),"input files")
    ;

  po::options_description cmdline_options;
  cmdline_options.add(generic).add(config).add(hidden);

  /*
    po::options_description config_file_options;
    config_file_options.add(config).add(hidden);
  */

  po::options_description visible("Allowed options");
  visible.add(generic).add(config);

  po::positional_options_description p;
  p.add("input-files", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).
	    options(cmdline_options).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << visible << "\n";
    return ret;
  }
  
  if (vm.count("version")) {
    std::cerr << "sysreplayer version 1.0" << "\n";
    return ret;
  }

  if (vm.count("warn")){
    warn_level = vm["warn"].as<int>();
    if(warn_level > ABORT_MODE){
      std::cout << "Wrong value for warn option" << std::endl;  
      return EXIT_FAILURE;}
  }

  if (vm.count("verify")) {
    verify = true;
  }

  if (vm.count("verbose")) {
    verbose = true;
  }
  
  if (vm.count("pattern")){
    pattern_data = vm["pattern"].as<std::string>();
  }
  
  if (vm.count("input-files")) {
    input_files = vm["input-files"].as<std::vector<std::string> >();
  } else {
    std::cout << "No dataseries input files.\n";
    ret = EXIT_FAILURE;
    return ret;
  }

  /*
   * This reads all extents of system call
   * operations from a set of DataSeries files.
   */
  const std::string kExtentTypePrefix = "IOTTAFSL::Trace::Syscall::";

  std::vector<std::string> system_calls;
  system_calls.push_back("open");
  system_calls.push_back("close");
  system_calls.push_back("read");
  system_calls.push_back("write");

  std::vector<TypeIndexModule *> type_index_modules;

  for (unsigned int i = 0; i < system_calls.size(); i++) {
    TypeIndexModule *type_index_module = new TypeIndexModule(kExtentTypePrefix + system_calls[i]);
    type_index_modules.push_back(type_index_module);
  }

  /* Specify to read dataseries input files */
  for (std::vector<std::string>::iterator iter = input_files.begin();
       iter != input_files.end();
       iter++) {
    for (unsigned int j = 0; j < type_index_modules.size(); j++) {
      type_index_modules[j]->addSource(*iter);
    }
  }

  std::vector<PrefetchBufferModule *> prefetch_buffer_modules;
  for (unsigned int i = 0; i < type_index_modules.size(); ++i) {
    /* Parallel decompress and replay, 64MiB buffer */
    PrefetchBufferModule *module = new PrefetchBufferModule(*(type_index_modules[i]), 64 * 1024 * 1024);
    prefetch_buffer_modules.push_back(module);
  }

  OpenSystemCallTraceReplayModule *open_replayer = new OpenSystemCallTraceReplayModule(*prefetch_buffer_modules[0],
										       verbose,
										       warn_level);
  CloseSystemCallTraceReplayModule *close_replayer = new CloseSystemCallTraceReplayModule(*prefetch_buffer_modules[1],
											  verbose,
											  warn_level);
  ReadSystemCallTraceReplayModule *read_replayer = new ReadSystemCallTraceReplayModule(*prefetch_buffer_modules[2],
										       verbose,
										       verify,
										       warn_level);
  WriteSystemCallTraceReplayModule *write_replayer = new WriteSystemCallTraceReplayModule(*prefetch_buffer_modules[3],
											  verbose,
											  verify,
											  warn_level,
											  pattern_data);

  std::vector<SystemCallTraceReplayModule *> system_call_trace_replay_modules;
  system_call_trace_replay_modules.push_back(open_replayer);
  system_call_trace_replay_modules.push_back(close_replayer);
  system_call_trace_replay_modules.push_back(read_replayer);
  system_call_trace_replay_modules.push_back(write_replayer);

  // Define a min heap that stores each module. The heap is ordered by time_called field.
  std::priority_queue<SystemCallTraceReplayModule*, std::vector<SystemCallTraceReplayModule*>, LessThanByTimeCalled> replayers_heap;
  SystemCallTraceReplayModule *execute_replayer = NULL;

  // Add all the modules to min heap if the module has extents
  for (unsigned int i = 0; i < system_call_trace_replay_modules.size(); ++i) {
    SystemCallTraceReplayModule *module = system_call_trace_replay_modules[i];
    // getSharedExtent() == NULL means that there are no extents in the module.
    if (module->getSharedExtent()) {
      replayers_heap.push(module);
    }
  }
  
  // Process all the records in the dataseries
  while(!replayers_heap.empty()) {
    // Get a module that has min time_called
    execute_replayer = replayers_heap.top();
    replayers_heap.pop();
    // Replay the operation that has min time_called
    execute_replayer->execute();
    // Check to see if all the extents in the module are processed
    if (execute_replayer->has_more_trace() || execute_replayer->getSharedExtent() != NULL){
      // No, there are more extents, so we add it to min_heap
      replayers_heap.push(execute_replayer);
    }
  }

  return ret;
}
