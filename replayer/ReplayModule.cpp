
/*
 * Copyright (c) 2011 Jack Ma
 * Copyright (c) 2011 Vasily Tarasov
 * Copyright (c) 2011 Koundinya Santhosh Kumar
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
#include <algorithm>
#include <vector>
#include <set>
#include <string.h>
#include <DataSeries/PrefetchBufferModule.hpp>
#include <DataSeries/TypeIndexModule.hpp>
#include <DataSeries/RowAnalysisModule.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include "Commander.hpp"

#define BUFFER_SIZE 1024 * 1024 * 1024
#define BLOCK_SIZE 512

class BlocktraceReplayModule : public RowAnalysisModule {
public:
	BlocktraceReplayModule(DataSeriesModule &source,
		     std::map<std::string,std::vector<uint64_t> > &config,
		     Commander *cmder)
		: RowAnalysisModule(source),
		  configuration(config),
		  processedFlag(false),
		  commander(cmder),
		  process_id(series, "process_id"),
		  operation(series, "operation"),
		  offset(series, "offset"),
		  request_size(series, "request_size"),
		  enter_time(series,"enter_time")
	{
		/* compute affine transformation for each
				field then store it in transform */
		for (std::map<std::string,
			std::vector<uint64_t> >::iterator iter = config.begin();
			iter != config.end(); iter++) {

			std::string fieldname = iter->first;
			double rMin = config[fieldname][0];
			double rMax = config[fieldname][1];
			double pMin = config[fieldname][2];
			double pMax = config[fieldname][3];
			double scale = (pMax - pMin + 1) / (rMax - rMin + 1);
			double trans = pMin - rMin * scale;

			/* first is the scale factor,
				second is the translation factor */
			std::vector<double> vec;
			vec.push_back(scale);
			vec.push_back(trans);
			transform[fieldname] = vec;
		}
	}

	void processRow()
	{
		if ((offset.val() + request_size.val()) > BUFFER_SIZE) {
			std::cout << enter_time.val()
				  << ": operation excceeds BUFFER_SIZE. "
				  << "Ignore operation."
				  << std::endl;
			return;
		}

		/* compute transformed values of each field */
		playFlag = true;
		uint64_t enter_time_val =
				findVal(enter_time.val(), "enter_time");

		/* If the replay parameter indicates to stop playing, exit */
		if (!playFlag && processedFlag)
			exitModule();

		uint64_t operation_val =
				findVal(operation.val(), "operation");
		uint64_t offset_val =
				findVal(offset.val(), "offset");
		uint64_t request_size_val =
				findVal(request_size.val(), "request_size");

		/* if all parameters are in range, execute the operation */
		if (playFlag) {
			commander->execute(operation_val,
					   enter_time_val,
					   offset_val * BLOCK_SIZE,
					   request_size_val);
			processedFlag = true;
		}
	}

	void completeProcessing()
	{
		commander->cleanup();
	}

	void exitModule()
	{
		printResult();
		exit(0);
	}

	void printResult()
	{
		commander->writeStats();
	}

private:
	/* compute the transformed value based on specified transformation */
	uint64_t findVal(uint64_t value, std::string fieldname)
	{
		std::vector<uint64_t> fieldConfig = configuration[fieldname];
		if (value < fieldConfig[0] || value > fieldConfig[1])
			playFlag = false;

		std::vector<double> fieldTransform = transform[fieldname];
		return (uint64_t) (((double) value) *
		       fieldTransform[0] +
		       fieldTransform[1]);
	}

	std::map<std::string,std::vector<double> > transform;
	std::map<std::string,std::vector<uint64_t> > configuration;
	bool playFlag;
	bool processedFlag;
	Commander *commander;

	/* DataSeries Block Trace Fields */
	Int32Field process_id;
	ByteField operation;
	Int64Field offset;
	Int64Field request_size;
	Int64Field enter_time;
};

int main(int argc, char *argv[])
{
	std::string usage =
		"Usage: "
			"replay "
			"[-v] "
			"-sync|-async "
			"device_id "
			"<configuration file> "
			"<dataseries files...>\n";

	if (strcmp(argv[1],"-v") == 0) {
		if (argc < 6) {
			std::cout << usage;
			return 1;
		}
	} else {
		if (argc < 5) {
			std::cout << usage;
			return 1;
		}
	}

	int argnum = 1;

	/* check if verbose mode is enabled */
	bool verbose = false;
	if (strcmp(argv[argnum], "-v") == 0) {
		verbose = true;
		argnum++;
	}

	/* calculate buffer placement to allign with page. Floor division. */
	void *mbuffer = malloc(BUFFER_SIZE);
	void *buffer((void*)(((int)(mbuffer) + 4096 - 1) / 4096 * 4096));

	/* create a Commander that executes operations */
	Commander *commander;
	if (strcmp(argv[argnum], "-sync") == 0) {
		argnum++;
		commander = new SynchronousCommander(argv[argnum],
						     buffer,
						     verbose);
		argnum++;
	} else if (strcmp(argv[argnum], "-async") == 0) {
		argnum++;
		commander = new AsynchronousCommander(argv[argnum],
						      buffer,
						      verbose);
		argnum++;
	} else {
		std::cout << usage;
		return 1;
	}

	/* Read in the configuration file */
	std::ifstream input;
	input.open(argv[argnum]);
	argnum++;

	std::string line;
	std::map<std::string,std::vector<uint64_t> > configuration;

	while (getline(input, line)) {
		std::vector<std::string> split_data;
		boost::split(split_data, line, boost::is_any_of(","));

		std::vector<uint64_t> configuration_data;
		for (std::vector<std::string>::iterator
						iter = ++split_data.begin();
						iter != split_data.end();
						iter++)
			configuration_data.push_back(
						(uint64_t)atof(iter->c_str()));

		if (configuration_data.size() != 4) {
			std::cout << "Illegal configuration file" << std::endl;
			return 1;
		}

		configuration[split_data[0]] = configuration_data;
	}

	if (configuration.size() == 0) {
		std::cout << "Illegal configuration file" << std::endl;
		return 1;
	}

	input.close();

	/* Specify to read in the extent type "read_write" */
	TypeIndexModule source("read_write");

	for (int i = argnum; i < argc; i++)
		source.addSource(argv[i]);

	/* Parallel decompress and stats, 64MiB buffer */
	PrefetchBufferModule prefetch(source, 64 * 1024 * 1024);
	BlocktraceReplayModule replayer(prefetch, configuration, commander);

	replayer.getAndDelete();
	replayer.printResult();

	free(mbuffer);
	delete commander;

	return 0;
}
