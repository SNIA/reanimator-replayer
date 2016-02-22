/*
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
 * pre-processor converts various units in different block trace formats
 * to the units defined by the SNIA standard. E.g., time is converted to Tfracs,
 * offset and I/O size to bytes, etc.
 *
 * This tool is usually invoked by a trace-specific convertor (e.g.,
 * blktrace2ds) after some preliminary conversion is already done. The reason
 * why unit conversion is performed in a separate tool (but not in the shell) is
 * speed.
 *
 * The tool produces a new CSV file that is then fed to a csv2ds-extra tool.
 *
 * Usage: pre-processor <trace_type> <input_file> <output_file>
 * 	  Recognized trace_types: spc, blk, vss, mps
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <boost/algorithm/string.hpp>

#define OPERATION_READ		0
#define OPERATION_WRITE		1

#define SECTOR_SIZE			(512)

#define MICRO_MULTIPLIER	(1000ULL * 1000ULL)
#define NANO_MULTIPLIER 	(MICRO_MULTIPLIER * 1000ULL)

using namespace std;

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
	cerr << "Usage:   " << pgmname << " <blk | spc | vss | mps> <input file> "
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
