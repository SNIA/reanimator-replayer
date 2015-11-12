/*
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2011-2012 Santhosh Kumar Koundinya
 * Copyright (c) 2011-2012 Jack Ma
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2011-2012 Erez Zadok
 * Copyright (c) 2011-2012 Geoff Kuenning
 * Copyright (c) 2011-2012 Stony Brook University
 * Copyright (c) 2011-2012 Harvey Mudd College
 * Copyright (c) 2011-2012 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * pre-processor converts various units in different block/system call trace formats
 * to the units defined by the SNIA standard. E.g., time is converted to Tfracs,
 * offset and I/O size to bytes, etc.
 *
 * This tool is usually invoked by a trace-specific convertor (e.g.,
 * blktrace2ds, systrace2ds) after some preliminary conversion is 
 * already done. The reason why unit conversion is performed in a
 * separate tool (but not in the shell) is speed.
 *
 * The tool produces a new CSV file that is then fed to a csv2ds-extra tool.
 *
 * Usage: pre-processor <trace_type> <input_file> <output_file>
 * 	  Recognized trace_types: spc, blk, vss, mps, sys
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp> 
#include <boost/tokenizer.hpp>

#define OPERATION_READ		0
#define OPERATION_WRITE		1

#define SECTOR_SIZE			(512)

#define MICRO_MULTIPLIER	(1000ULL * 1000ULL)
#define NANO_MULTIPLIER 	(MICRO_MULTIPLIER * 1000ULL)

using namespace std;
bool sysProcessOpen(const vector<string> &arguments, string &field);

bool spcProcessRow(const string &inRow, string &outRow)
{
	vector<string> fields;
	boost::split(fields, inRow, boost::is_any_of(","));

	if (fields.size() < 5) {
		clog << "SPC: Malformed record: '" << inRow << "'. Too few columns.\n";
		return false;
	}

	/* The input expected by csv2ds-extra is <extent name>, <fields>.
	 *
	 * The input fields are ordered as follows.
	 * ASU, LBA, Size, Opcode, Timestamp, Extra fields . . .
	 * Example: 1,240840,3072,w,0.026214
	 *
	 * Consult the SPC Trace format specification document for more details.
	 *
	 * TODO: Sanity check all field values (say timestamp > 0) and return
	 * appropriate errors.
	 */
	const char *extentName = "read_write";
	string &asu = fields[0];

	/* There's no clarity on what the block size is. We are assuming that it
	 * equals sector size (512). */
	uint64_t offset = (uint64_t) atoll(fields[1].c_str()) * SECTOR_SIZE;

	string &size = fields[2];
	int opcode = (fields[3] == "R" || fields[3] == "r") ? OPERATION_READ :
			OPERATION_WRITE;
	/* DataSeries expects the time in Tfracs. One tfrac is 1/(2^32) of a
	 * second */
	uint64_t timestamp = (uint64_t)(atof(fields[4].c_str()) *
			(((uint64_t)1)<<32));

	stringstream formattedRow;
	formattedRow << extentName << "," << asu << "," << offset << "," << size
			<< "," << opcode << "," << timestamp;

	outRow = formattedRow.str();

	return true;
}

bool sysProcessRow(const string &inRow, string &outRow)
{
	// Check to see if it reaches end of trace file.
	if (inRow.find("exited") != string::npos)
	  return true;
	
	/* The input expected by csv2ds-extra is <extent name>, <fields>.
	*
	* The input fields are ordered as follows.
	* relative_timestamp syscall(args) = return_val
	* Example: 0.000061 close(3)                  = 0
	*
	* Consult the SPC Trace format specification document and 
	* strace man page for more details.
	*
	*/

	/* Split return information and system call information.
	 * The input fields are ordered as follows.
	 * relative_timestamp syscall(args) = return_val
	 */
	vector<string> fields;
	boost::algorithm::split_regex(fields, inRow, boost::regex( " *= " ));

	// Check to make sure the trace record is valid.
	if (fields.size() < 2) {
	  clog << "SYS: Malformed record: '" << inRow << "'. Too few columns.\n";
	  return false;
	}
	
	// Get system call information and return information
	string sys_call_info = fields[0];
	string ret_info = fields[1];

	// Eliminate leading and trailing spaces
	boost::trim(sys_call_info);
	boost::trim(ret_info);
	
	// Split system call arguments and name
	size_t left_paren_index = sys_call_info.find_first_of("(");
	size_t right_paren_index = sys_call_info.find_first_of(")");
	if (left_paren_index == std::string::npos || right_paren_index == std::string::npos) {
	  clog << "SYS: Malformed record: '" << sys_call_info << "'. ( or ) is missing.\n";
	  return false;
	}
	
	// Get system call time and name
	string sys_call_time_and_name = sys_call_info.substr(0, left_paren_index);
	// Get system call arguments
	// We don't want "(" and ")"
	string sys_call_args = sys_call_info.substr(left_paren_index + 1, right_paren_index - left_paren_index - 1);
		
	// Split system call name and time stamp
	boost::split(fields, sys_call_time_and_name, boost::is_any_of(" \t"), boost::token_compress_on);
	string time_called = fields[0];
	string sys_call_name = fields[1];
	
	if (sys_call_name == "open") {
	  boost::tokenizer<boost::escaped_list_separator<char> > args_tokenizer(sys_call_args);
	  std::vector<string> arguments;
	  arguments.assign(args_tokenizer.begin(), args_tokenizer.end());
	  if (arguments.size() < 2) {
	    clog << "SYS: Malformed record: '" << sys_call_info << "'. ( is missing.\n";
	    return false;
	  } 
	  //else if (arguments.size() == 2){
	  //  sys_call_name = "open2";
	  //} else if (arguments.size() == 3){
	  //  sys_call_name = "open3";
	  //}
	  if (sysProcessOpen(arguments, sys_call_args) == false) {
	    return false;
	  }
	}
	
	/* Right now we don't need to worry about this. So comment it out.
	 * DataSeries expects the time in Tfracs. One tfrac is 1/(2^32) of a
	 * second 
	uint64_t rel_timestamp = (uint64_t)(atof(fields[0].c_str()) *
					    (((uint64_t)1)<<32));
	// Make sure timestamp is valid.
	if (rel_timestamp < 0) {
	  clog << "SYS: Malformed relative timestamp: '" << fields[0] << "'.";
	  clog << "Timestamp less than 0.\n";
	  return false;
	  }
	*/
	
	// Formatting output to csv2ds-extra
	stringstream formattedRow;
	formattedRow << sys_call_name << "," << time_called << "," << ret_info  << "," <<sys_call_args;
	outRow = formattedRow.str();
	return true;
}

bool sysProcessOpen(const vector<string> &arguments, string &csv_sys_call_args) {
  string path_name = arguments[0];
  string flags_arg = arguments[1];  
  // Eliminate leading and trailing spaces
  boost::trim_if(flags_arg, boost::is_any_of(" "));

  vector<string> flags;
  boost::split(flags, flags_arg, boost::is_any_of("|"), boost::token_compress_on);
  
  std::unordered_map<string, bool> flag_map;

  vector<int> flags_args;
  for (int i = 0; i < 18; i++){
    flags_args.push_back(0);
  }
  
  bool flag_read_only = false;
  bool flag_write_only = false;
  bool flag_read_and_write = false;
  bool flag_append = false;
  bool flag_async = false;
  bool flag_create = false;
  bool flag_direct = false;
  bool flag_directory = false;
  bool flag_exclusive = false;
  bool flag_largefile = false;
  bool flag_no_access_time = false;
  bool flag_no_controlling_terminal = false;
  bool flag_no_follow = false;
  bool flag_no_blocking_mode = false;
  bool flag_no_delay = false;
  bool flag_synchronous = false;
  bool flag_truncate = false;
  bool flag_close_on_exec = false;

  flag_map["O_RDONLY"] = 0;
  flag_map["O_WRONLY"] = 0;
  flag_map["O_RDWR"] = 0;
  flag_map["O_APPEND"] = 0;
  flag_map["O_ASYNC"] = 0;
  flag_map["O_CREAT"] = 0;
  flag_map["O_DIRECT"] = 0;
  flag_map["O_DIRECTORY"] = 0;
  flag_map["O_EXCL"] = 0;
  flag_map["O_LARGEFILE"] = 0;

  for (string flag : flags) {
    if (flag.compare("O_RDONLY") == 0) {
      flag_read_only = true;
    } else if (flag.compare("O_WRONLY") == 0) {
      flag_write_only = true;
    } else if (flag.compare("O_RDWR") == 0) {
      flag_read_and_write = true;
    } else if (flag.compare("O_APPEND") == 0) {
      flag_append = true;
    } else if (flag.compare("O_ASYNC") == 0) {
      flag_async = true;
    } else if (flag.compare("O_CREAT") == 0) {
      flag_create = true;
    } else if (flag.compare("O_DIRECT") == 0) {
      flag_direct = true;
    } else if (flag.compare("O_DIRECTORY") == 0) {
      flag_directory = true;
    } else if (flag.compare("O_EXCL") == 0) {
      flag_exclusive = true;
    } else if (flag.compare("O_LARGEFILE") == 0) {
      flag_largefile = true;
    } else if (flag.compare("O_NOATIME") == 0) {
      flag_no_access_time = true;
    } else if (flag.compare("O_NOCTTY") == 0) {
      flag_no_controlling_terminal = true;
    } else if (flag.compare("O_NOFOLLOW") == 0) {
      flag_no_follow = true;
    } else if (flag.compare("O_NONBLOCK") == 0) {
      flag_no_blocking_mode = true;
    } else if (flag.compare("O_NDELAY") == 0) {
      flag_no_delay = true;
    } else if (flag.compare("O_SYNC") == 0) {
      flag_synchronous = true;
    } else if (flag.compare("O_TRUNC") == 0) {
      flag_truncate = true;
    } else if (flag.compare("O_CLOEXEC") == 0){
      flag_close_on_exec = true;
    } else {
      clog << "SYS: Malformed open system call: '" << flags_arg << "'." << endl;
      clog << "Unknown flag: '" << flag << "'.\n";
      return false;
    }
  }
  // Formatting flags.
  stringstream csv_args_stream;
  csv_args_stream << path_name << "," << flag_read_only << "," << flag_write_only << "," << flag_read_and_write << "," 
		  << flag_append << "," << flag_async << "," << flag_create << ","
		  << flag_direct << "," << flag_directory << "," << flag_exclusive << ","
		  << flag_largefile << "," << flag_no_access_time << "," << flag_no_controlling_terminal << "," 
		  << flag_no_follow << "," << flag_no_blocking_mode << "," << flag_no_delay << "," 
		  << flag_synchronous << "," << flag_truncate << "," << flag_close_on_exec;

  if (arguments.size() == 3) {


  }

  csv_sys_call_args = csv_args_stream.str();
  return true;
}

bool blkProcessRow(const string &inRow, string &outRow)
{
	vector<string> fields;
	boost::split(fields, inRow, boost::is_any_of(","));

	if (fields.size() < 7) {
		clog << "BLK: Malformed record: '" << inRow << "'. Too few columns."
				"\n";
		return false;
	}

	/*
	 * Input rows are formatted as follows:
	 * <extent name>,<timestamp in nanos>,<process id>,<device id>,
	 * <operation code>,<offset in sectors>,<request size in sectors>
	 *
	 * Example: read_write,0001145250,243,81,WS,25168280,4096
	 */

	const string &extent_name = fields[0];
	uint64_t timestamp = (uint64_t)((atof(fields[1].c_str()) /
				NANO_MULTIPLIER) * (((uint64_t)1)<<32));
	const string &processId = fields[2];
	const string &deviceId = fields[3];
	int opcode = (fields[4][0] == 'R') ? OPERATION_READ : OPERATION_WRITE;
	int sync_flag = (fields[4].find('S') == string::npos) ? 0 : 1;
	uint64_t offset = (uint64_t)atoll(fields[5].c_str()) * SECTOR_SIZE;
	const string &requestSize = fields[6];

	stringstream s;
	s << extent_name << "," << timestamp << "," << processId << "," << deviceId
			<< "," << opcode << "," << offset << "," << requestSize
			<< "," << sync_flag;

	outRow = s.str();

	return true;
}

bool vssProcessRow(const string &inRow, string &outRow)
{
	vector<string> fields;
	boost::split(fields, inRow, boost::is_any_of(","));

	if (fields.size() != 6) {
		clog << "VSS: Malformed record: '" << inRow << "'. Incorrect number of"
				" columns.\n";
		return false;
	}

	/* The input is as follows:
	 *
	 * Vscsi Cmd Trace. (Trace Format Version 1)
	 * Serial Number,IO Data Length,Num SG Entries,Command Type,LBN,Time Stamp
	 *   (microseconds)
	 * 2148401256,4096,1,read,12312,4332930811158
	 * 2148401193,4096,1,write,0,4332933099966
	 *
	 * The following fields are ignored: Serial Number, Num SG Entries.
	 *
	 * TODO: Sanity check all field values (say timestamp > 0) and return
	 * appropriate errors.
	 */
	const char *extentName = "read_write";
	string &size = fields[1];
	int opcode = (fields[3] == "read") ? OPERATION_READ : OPERATION_WRITE;

	/* Assume a block size of 512. */
	uint64_t offset = (uint64_t) atoll(fields[4].c_str()) * SECTOR_SIZE;

	/* DataSeries expects the time in Tfracs. One tfrac is 1/(2^32) of a
	 * second */
	uint64_t timestamp = (uint64_t)(atof(fields[5].c_str()) / MICRO_MULTIPLIER
			 * (((uint64_t)1)<<32));

	stringstream formattedRow;
	formattedRow << extentName << "," << size << "," << opcode << "," << offset
			<< "," << timestamp;

	outRow = formattedRow.str();

	return true;
}

bool mpsProcessRow(const string &inRow, string &outRow)
{
	vector<string> fields;
	boost::split(fields, inRow, boost::is_any_of(","));

	if (fields.size() != 15) {
		clog << "MPST: Malformed record: '" << inRow << "'. Incorrect number of"
				" columns.\n";
		return false;
	}

	/* Input is of the following format:	 *
	 * <"DiskRead" | "DiskWrite", <TimeStamp>, <Process Name ( PID)>,
	 * <ThreadID>, <IrpPtr>, <ByteOffset>, <IOSize>, <ElapsedTime>, <DiskNum>,
	 * <IrpFlags>, <DiskSvcTime>, <I/O Pri>, <VolSnap>, <FileObject>, <FileName>
	 *
	 * Output should be:
	 * "read_write", <enter_time>, <exit_time>, <process_id>, <operation>,
	 * <offset>, <request_size in bytes>
	 */

	const char *extentName = "read_write";
	uint64_t enterTime = (uint64_t)(atof(fields[1].c_str()) / MICRO_MULTIPLIER
			 * (((uint64_t)1)<<32));
	uint64_t exitTime = (uint64_t)(atof(fields[7].c_str()) / MICRO_MULTIPLIER
			 * (((uint64_t)1)<<32)) + enterTime;
	uint32_t pid = atoi(strchr(fields[2].c_str(), '(') + 1);
	uint8_t operation = (fields[0].find("DiskRead") != string::npos) ?
			OPERATION_READ : OPERATION_WRITE;
	uint64_t offset = atoll(fields[5].c_str());
	uint64_t requestSize = atoll(fields[6].c_str());

	stringstream formattedRow;
	formattedRow << extentName << "," << enterTime << "," << exitTime << "," <<
			pid << "," << operation << "," << offset << "," << requestSize;

	outRow = formattedRow.str();

	return true;
}

void showUsage(const char *pgmname)
{
	cerr << "Usage:   " << pgmname << " <blk | spc | vss | mps | sys> <input file> "
			"<output file>\n";
	cerr << "Example: " << pgmname << " spc /foo/bar /foo/baz\n";
}

int main(int argc, char *argv[])
{
	const char *traceType;
	const char *inFileName;
	const char *outFileName;
	ifstream inFile;
	ofstream outFile;
	string inRow;
	string outRow;
	bool (*processRow)(const string &inRow, string &outRow) = NULL;
	int rowNum;
	int ret = EXIT_SUCCESS;

	if (argc < 4) {
		showUsage(argv[0]);
		return EXIT_FAILURE;
	}

	traceType = argv[1];
	inFileName = argv[2];
	outFileName = argv[3];

	if (!strcmp("spc", traceType))
		processRow = spcProcessRow;
	else if (!strcmp("blk", traceType))
		processRow = blkProcessRow;
	else if (!strcmp("vss", traceType))
		processRow = vssProcessRow;
	else if (!strcmp("mps", traceType))
		processRow = mpsProcessRow;
	else if (!strcmp("sys", traceType))
	        processRow = sysProcessRow;
	else {
		cerr << "Unsupported trace type '" << traceType << "'.\n";
		showUsage(argv[0]);
		ret = EXIT_FAILURE;
		goto cleanup;
	}

	inFile.open(inFileName, ios_base::in);
	if (!inFile.is_open()) {
		cerr << "Unable to open input file '" << inFileName << "'.\n";
		ret = EXIT_FAILURE;
		goto cleanup;
	}

	outFile.open(outFileName, ios_base::out);
	if (!outFile.is_open()) {
		cerr << "Unable to open output file '" << outFileName << "'.\n";
		ret = EXIT_FAILURE;
		goto cleanup;
	}

	rowNum = 0;
	while(getline(inFile, inRow)) {
		rowNum++;
		if (!processRow(inRow, outRow)) {
			cerr << "Error processing record " << rowNum << ".\n";
			ret = EXIT_FAILURE;
			continue;
		}

		outFile << outRow << "\n";
	}

cleanup:
	if (inFile.is_open()) {
		inFile.close();
	}

	if (outFile.is_open()) {
		outFile.close();
	}

	/* Delete the out file if we are not successful */
	if (ret != EXIT_SUCCESS) {
		remove(outFileName); /* Don't handle errors here */
	}

	return ret;
}
