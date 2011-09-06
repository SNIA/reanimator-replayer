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

#define SECTOR_SIZE			(512)
#define NANOS_MULTIPLIER 	(1000 * 1000 * 1000)

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
	int opcode = (fields[3] == "R" || fields[3] == "r") ? 0 : 1;
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
			NANOS_MULTIPLIER) * (((uint64_t)1)<<32));
	const string &processId = fields[2];
	const string &deviceId = fields[3];
	int opcode = (fields[4][0] == 'R') ? 0 : 1;
	const string &offset = fields[5];
	const string &requestSize = fields[6];

	stringstream s;
	s << extent_name << "," << timestamp << "," << processId << "," << deviceId
			<< "," << opcode << "," << offset << "," << requestSize;

	outRow = s.str();

	return true;
}

void showUsage(const char *pgmname)
{
	cerr << "Usage:   " << pgmname << " <blk | spc> <input file> <output file>\n";
	cerr << "Example: " << pgmname << " spc /foo/bar /foo/baz\n";
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	int rowNum;
	string inRow, outRow;
	bool (*processRow)(const string &inRow, string &outRow);

	if (argc < 4) {
		showUsage(argv[0]);
		return 1;
	}

	processRow = !strcmp("spc", argv[1]) ? spcProcessRow : blkProcessRow;
	const char *inFileName = argv[2];
	const char *outFileName = argv[3];

	ifstream inFile(inFileName, ios_base::in);
	ofstream outFile(outFileName, ios_base::out);

	if (!inFile.is_open()) {
		cerr << "Unable to open input file '" << inFileName << "'.\n";
		goto cleanup;
	}

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
