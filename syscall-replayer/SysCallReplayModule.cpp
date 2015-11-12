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

class SystemCallTraceReplayModule : public RowAnalysisModule {
protected:
  bool verbose_;
  bool compare_retval_;
  DoubleField time_called_;
  Int64Field return_value_;
public:
  SystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag, bool compare_retval_flag) : 
    RowAnalysisModule(source),
    verbose_(verbose_flag),
    compare_retval_(compare_retval_flag),
    time_called_(series, "time_called"),
    return_value_(series, "return_value", Field::flag_nullable) {
  }

  double time_called() {
    return (double)time_called_.val();
  }

  Extent::Ptr getSharedExtent() {
    Extent::Ptr e = source.getSharedExtent();
    if (e == NULL) {
      completeProcessing();
      return e;
    }
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
      std::cout << "Captured return value" << ret_val << ", " << std::endl;
      std::cout << "Replayed return value" << return_value_.val() << std::endl;
    }

    if (compare_retval_ == true && return_value_.val() != ret_val) {
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

  uint32_t getOpenFlags() {
    int sum = flag_read_only.val() + flag_write_only.val() + flag_read_and_write.val();
    if (sum != 0) {
      // FIXME!!!!! ERROR
    }
    uint32_t flags = O_RDONLY;
    if (flag_read_only.val() == 1) {
      flags = O_RDONLY;
    } else if (flag_write_only.val() == 1) {
      flags = O_WRONLY;
    } else if (flag_write_only.val() == 1) {
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

  mode_t getOpenMode() {
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
			
    if (mode == 0 && O_CREAT == 0) {
      // FIX ME!!!!!! ERROR
    }
    return mode;
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
  }

public:
  OpenSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag, bool compare_retval_flag) : 
    SystemCallTraceReplayModule(source, verbose_flag, compare_retval_flag),
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
    uint32_t flags = getOpenFlags();
    mode_t mode = getOpenMode();

    if (verbose_) {
      std::cout.precision(25);
      std::cout << "time called:" << std::fixed << time_called() << std::endl;
      std::cout << "pathname:" << pathname << std::endl;
      std::cout << "flags:" << flags << std::endl;
      std::cout << "mode:" << mode << std::endl;
    }
			
    int ret = open(pathname, flags, mode);
    compare_retval(ret);

    if (ret == -1) {
      perror(pathname);
    } else {
      std::cout << given_pathname.val() << " is successfully opened..." << std::endl;
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
  CloseSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag, bool compare_retval_flag) : 
    SystemCallTraceReplayModule(source, verbose_flag, compare_retval_flag),
    descriptor_(series, "descriptor"){
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
    if (verbose_) {
      std::cout.precision(25);
      std::cout << "time called:" << std::fixed << time_called() << std::endl;
      std::cout << "descriptor:" << descriptor_.val() << std::endl;
    }
			
    int ret = close(descriptor_.val());
    compare_retval(ret);

    if (ret == -1) {
      perror("close");
    } else {
      std::cout << "fd " << descriptor_.val() << " is successfully closed..." << std::endl;
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
  ReadSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag, bool verify_flag, bool compare_retval_flag) : 
    SystemCallTraceReplayModule(source, verbose_flag, compare_retval_flag),
    verify_(verify_flag),
    descriptor_(series, "descriptor"), 
    data_read_(series, "data_read", Field::flag_nullable), 
    bytes_requested_(series, "bytes_requested") {
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
    if (verbose_) {
      std::cout.precision(23);
      std::cout << "time called:" << std::fixed << time_called() << std::endl;
      std::cout << "descriptor:" << descriptor_.val() << std::endl;
    }
    char buffer[bytes_requested_.val()];
    int ret = read(descriptor_.val(), buffer, bytes_requested_.val());
    compare_retval(ret);

    if (verify_ == true) {
      if (memcmp(data_read_.val(),buffer,ret) != 0){
	// data aren't same
	std::cerr << "Verification of data in read failed.\n";
	abort();
      } else {
	if (verbose_) {
	  std::cout << "Verification of data in read success.\n";
	}
      }
    }

    if (ret == -1) {
      perror("read");
    } else {
      std::cout << "read is executed successfully!" << std::endl;
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
				   bool compare_retval_flag, 
				   std::string pattern_data) : 
    SystemCallTraceReplayModule(source, verbose_flag, compare_retval_flag),
    verify_(verify_flag),
    pattern_data_(pattern_data),
    descriptor_(series, "descriptor"), 
    data_written_(series, "data_written", Field::flag_nullable), 
    bytes_requested_(series, "bytes_requested") {
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
    char *buffer = (char *)data_written_.val();
    if (verbose_) {
      std::cout << "write:";
      std::cout.precision(25);
      std::cout << "time called(" << std::fixed << time_called() << "),";
      std::cout << "descriptor(" << descriptor_.val() << "),";
      std::cout << "data(" << buffer << "),";
      std::cout << "nbytes(" << nbytes << ")" << std::endl;
    }
    
    if (!pattern_data_.empty()){
      buffer = new char[nbytes];
      if (pattern_data_ == "random") {
	random_file_.read(buffer, nbytes);
      } else {
	int pattern_hex;
	std::stringstream pattern_stream;
	pattern_stream << std::hex << pattern_data_;
	pattern_stream >> pattern_hex;
	memset(buffer, pattern_hex, nbytes);
	std::cout << "hex:" << pattern_hex << std::endl;
	std::cout << "buffer:" << buffer << std::endl;
      }
    }
        
    int ret = write(descriptor_.val(), buffer, nbytes);
    compare_retval(ret);
    
    if (!pattern_data_.empty()){
      delete[] buffer;
    }
    
    if (ret == -1) {
      perror("write");
    } else {
      std::cout << "write is successfully replayed\n";
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


int main(int argc, char *argv[]) {	
  namespace po = boost::program_options;

  int ret = EXIT_SUCCESS;
  bool verbose = false;
  bool verify = false;
  bool finished_replaying = false;	
  bool compare_retval = false;
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
    ("return", "compares return values of replayed system calls to return values captured by strace")
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
    //goto cleanup;
  }
  
  if (vm.count("version")) {
    std::cerr << "sysreplayer version 1.0" << "\n";
    return ret;
    //goto cleanup;
  }

  if (vm.count("return")) {
    compare_retval = true;
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
    //goto cleanup;
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
    PrefetchBufferModule *module = new PrefetchBufferModule(*(type_index_modules[i]), 64 * 1024 * 1024);
    prefetch_buffer_modules.push_back(module);
  }

  /* Parallel decompress and stats, 64MiB buffer */
  OpenSystemCallTraceReplayModule *open_replayer = new OpenSystemCallTraceReplayModule(*prefetch_buffer_modules[0],
										       verbose,
										       compare_retval);
  CloseSystemCallTraceReplayModule *close_replayer = new CloseSystemCallTraceReplayModule(*prefetch_buffer_modules[1],
											  verbose,
											  compare_retval);
  ReadSystemCallTraceReplayModule *read_replayer = new ReadSystemCallTraceReplayModule(*prefetch_buffer_modules[2],
										       verbose,
										       verify,
										       compare_retval);
  WriteSystemCallTraceReplayModule *write_replayer = new WriteSystemCallTraceReplayModule(*prefetch_buffer_modules[3],
											  verbose,
											  verify,
											  compare_retval,
											  pattern_data);
  std::vector<SystemCallTraceReplayModule *> system_call_trace_replay_modules;
  system_call_trace_replay_modules.push_back(open_replayer);
  system_call_trace_replay_modules.push_back(close_replayer);
  system_call_trace_replay_modules.push_back(read_replayer);
  system_call_trace_replay_modules.push_back(write_replayer);
  
  SystemCallTraceReplayModule *execute_replayer = NULL;

  std::priority_queue<SystemCallTraceReplayModule*, std::vector<SystemCallTraceReplayModule*>, LessThanByTimeCalled> sorted_replayers;
	
  while (!finished_replaying) {
    for (unsigned int i = 0; i < system_call_trace_replay_modules.size(); ++i) {
      SystemCallTraceReplayModule *module = system_call_trace_replay_modules[i];
      if (module->getSharedExtent()) {
	sorted_replayers.push(module);
      }
    }

    if (sorted_replayers.empty()) {
      finished_replaying = true;
      break;
    }

    while(!sorted_replayers.empty()) {
      execute_replayer = sorted_replayers.top();
      sorted_replayers.pop();
      execute_replayer->execute();
      if (execute_replayer->has_more_trace()){
	sorted_replayers.push(execute_replayer);
      }
    }

  }

  return ret;
}
