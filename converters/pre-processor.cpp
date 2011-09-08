/*
 * pre-processor.cpp
 *
 * Copyright (c) 2011 Koundinya Santhosh Kumar
 * Copyright (c) 2011 Jack Ma
 * Copyright (c) 2011 Vasily Tarasov
 * Copyright (c) 2011 Erez Zadok
 * Copyright (c) 2011 Geoff Kuenning
 * Copyright (c) 2011 Stony Brook University
 * Copyright (c) 2011 Harvey Mudd College
 * Copyright (c) 2011 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
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
		cerr << "SPC: Malformed record: '" << inRow << "'. Too few columns.\n";
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
	formattedRow << extentName << "," << asu << "," << offset << "," << size << "," << opcode << ","
			<< timestamp;

	outRow = formattedRow.str();

	return true;
}

bool blkProcessRow(const string &inRow, string &outRow)
{
	vector<string> fields;
	boost::split(fields, inRow, boost::is_any_of(","));

	if (fields.size() < 7) {
		cerr << "BLK: Malformed record: '" << inRow << "'. Too few columns."
				"\n";
		return false;
	}

	/*
	 * Input rows are formatted as follows:
	 * <extent name>, <timestamp in nanos>, <process id>, <device id>,
	 * <operation code>, <offset>, <request size>
	 *
	 * Example: read_write,0001145250,243,81,W,25168280,4096
	 */

	const string &extent_name = fields[0];
	uint64_t timestamp = (uint64_t)((atof(fields[1].c_str()) /
			NANO_MULTIPLIER) * (((uint64_t)1)<<32));
	const string &processId = fields[2];
	const string &deviceId = fields[3];
	int opcode = (fields[4][0] == 'R') ? OPERATION_READ : OPERATION_WRITE;
	const string &offset = fields[5];
	const string &requestSize = fields[6];

	stringstream s;
	s << extent_name << "," << timestamp << "," << processId << "," << deviceId
			<< "," << opcode << "," << offset << "," << requestSize;

	outRow = s.str();

	return true;
}

bool vssProcessRow(const string &inRow, string &outRow)
{
	vector<string> fields;
	boost::split(fields, inRow, boost::is_any_of(","));

	if (fields.size() != 6) {
		cerr << "VSS: Malformed record: '" << inRow << "'. Incorrect number of"
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
	uint64_t timestamp = (uint64_t)(atoll(fields[5].c_str()) / MICRO_MULTIPLIER
			 * (((uint64_t)1)<<32) );

	stringstream formattedRow;
	formattedRow << extentName << "," << size << "," << opcode << "," << offset
			<< "," << timestamp;

	outRow = formattedRow.str();

	return true;
}

void showUsage(const char *pgmname)
{
	cerr << "Usage:   " << pgmname << " <blk | spc | vss> <input file> <output file>\n";
	cerr << "Example: " << pgmname << " spc /foo/bar /foo/baz\n";
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	int rowNum;
	string inRow, outRow;
	bool (*processRow)(const string &inRow, string &outRow) = NULL;
	ifstream inFile;
	ofstream outFile;
	const char *traceType;
	const char *inFileName;
	const char *outFileName;

	if (argc < 4) {
		showUsage(argv[0]);
		return 1;
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
	else {
		cerr << "Unsupported trace type '" << traceType << "'.\n";
		goto cleanup;
	}

	inFile.open(inFileName, ios_base::in);
	if (!inFile.is_open()) {
		cerr << "Unable to open input file '" << inFileName << "'.\n";
		goto cleanup;
	}

	outFile.open(outFileName, ios_base::out);
	if (!outFile.is_open()) {
		cerr << "Unable to open output file '" << outFileName << "'.\n";
		goto cleanup;
	}

	rowNum = 0;
	while(getline(inFile, inRow)) {
		rowNum++;
		if (!processRow(inRow, outRow)) {
			cerr << "Error processing record " << rowNum << ".\n";
			continue;
		}

		outFile << outRow << "\n";
	}

	/* All is well */
	ret = EXIT_SUCCESS;

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
