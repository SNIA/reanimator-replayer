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

#define _LARGEFILE64_SOURCE	1

#include <iostream>
#include <string>
#include <sstream>

#include <algorithm>
#include <vector>
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
#include <getopt.h>

#include <stdexcept>

#include "Commander.hpp"
#include "ReplayStats.hpp"

#define BUFFER_SIZE	(1024 * 1024 * 1024)
#define PAGE_SIZE	(4096)
#define BLOCK_SIZE	(512)

/* The commander whose status we want to report. */
static ReplayStats *rs = NULL;

void reportProgress(int sig)
{
	if (rs) {
		rs->printStats(std::cout);
	} else {
		std::cout << "Nothing to report." << std::endl;
	}
}

class BlocktraceReplayModule : public RowAnalysisModule {
public:
	BlocktraceReplayModule(DataSeriesModule &source,
		     std::map<std::string,std::vector<uint64_t> > &config,
		     Commander *cmder, ReplayStats *replayStats, bool verboseFlag)
		: RowAnalysisModule(source),
		  configuration(config),
		  commander(cmder),
		  stats(replayStats),
		  verbose(verboseFlag),
		  process_id(series, "process_id", Field::flag_nullable),
		  operation(series, "operation"),
		  offset(series, "offset"),
		  request_size(series, "request_size"),
		  enter_time(series,"enter_time", Field::flag_nullable)
	{
		if (!commander) {
			throw std::invalid_argument("NULL Commander");
		}

		if (!stats) {
			throw std::invalid_argument("NULL ReplayStats");
		}

		/* compute affine transformation for each
				field then store it in transform */
		for (std::map<std::string,
			std::vector<uint64_t> >::iterator iter = config.begin();
			iter != config.end(); iter++) {

			std::string fieldname = iter->first;
			uint64_t rMin = config[fieldname][0];
			uint64_t rMax = config[fieldname][1];
			uint64_t pMin = config[fieldname][2];
			uint64_t pMax = config[fieldname][3];
			double scale = ((double)(pMax - pMin + 1)) / (rMax - rMin + 1);
			double trans = (double)pMin - (rMin * scale);

			/* first is the scale factor, second is the translation factor */
			std::vector<double> vec;
			vec.push_back(scale);
			vec.push_back(trans);
			transform[fieldname] = vec;
		}
	}

	void processRow()
	{
		/* Update the record timestamp of the last trace record to be
		 * replayed. The value is in Tfracs.*/
		stats->lastRecordTimestamp = enter_time.val();

		switch ((int)operation.val()) {
		case OPERATION_READ:
			stats->readRecords++;
			stats->readRecordsSize += request_size.val();
			break;

		case OPERATION_WRITE:
			stats->writeRecords++;
			stats->writeRecordsSize += request_size.val();
			break;

		default:
			/* Should never be here */
			if (verbose) {
				std::stringstream msg;
				msg << "unknown operation type " << (int)operation.val();
				dumpRow(std::clog, msg.str().c_str());
			}
			return;
		}

		/* Compute transformed values of each field. */
		uint64_t enter_time_val;
		uint64_t operation_val;
		uint64_t offset_val;
		uint64_t request_size_val;

		if (!findVal(enter_time.val(), "enter_time", &enter_time_val) ||
			!findVal(operation.val(), "operation", &operation_val) ||
			!findVal(offset.val(), "offset", &offset_val) ||
			!findVal(request_size.val(), "request_size", &request_size_val)) {
			return;
		}

		if (request_size_val > BUFFER_SIZE) {
			if (verbose) {
				std::stringstream msg;
				msg << "transformed request size (" << request_size_val << ")"
						" exceeds " << BUFFER_SIZE;
				dumpRow(std::clog, msg.str().c_str());
			}
			return;
		}

		/* Now convert time from Tfracs to nano-seconds. */
		enter_time_val = (uint64_t) (((double)enter_time_val) /
				(((uint64_t)1)<<32) * NANO_TIME_MULTIPLIER);

		/* All parameters are in range, execute the operation. */
		commander->execute(operation_val,
				   enter_time_val,
				   offset_val * BLOCK_SIZE,
				   request_size_val);
	}

	void completeProcessing()
	{
		commander->cleanup();
	}

	void printResult()
	{
		commander->getReplayStats()->printStats(std::cout);
	}

private:
	/* compute the transformed value based on specified transformation */
	bool findVal(uint64_t value, std::string fieldname,
			uint64_t *transformedValue)
	{
		std::vector<uint64_t> fieldConfig = configuration[fieldname];
		if (value < fieldConfig[0] || value > fieldConfig[1]) {
			if (verbose) {
				std::stringstream msg;
				msg << fieldname << ": value (" << value << ") is out of range"
						" [" << fieldConfig[0] << "," << fieldConfig[1] << "]";
				dumpRow(std::cerr, msg.str().c_str());
			}
			return false;
		}

		std::vector<double> fieldTransform = transform[fieldname];
		*transformedValue =  (uint64_t) (value * fieldTransform[0] +
				fieldTransform[1]);
		return true;
	}

	void dumpRow(std::ostream &out, const char *msg)
	{
		out << process_id.val() << "," << (int)operation.val() << "," <<
			offset.val() << "," << request_size.val() << "," << enter_time.val()
			<< ": " << msg << ".\n";
	}

	std::map<std::string,std::vector<double> > transform;
	std::map<std::string,std::vector<uint64_t> > configuration;
	Commander *commander;
	ReplayStats *stats;
	bool verbose;

	/* DataSeries Block Trace Fields */
	Int32Field process_id;
	ByteField operation;
	Int64Field offset;
	Int64Field request_size;
	Int64Field enter_time;
};

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	bool verbose = false;
	std::string mode;
	std::string device_id;
	std::string config_file;
	std::vector<std::string> input_files;
	Commander *commander = NULL;
	void *mbuffer = NULL;
	void *buffer = NULL;
	int rowNum;
	std::string line;
	std::map<std::string,std::vector<uint64_t> > configuration;
	TypeIndexModule source("read_write");
	std::ifstream input;
	PrefetchBufferModule *prefetch = NULL;
	BlocktraceReplayModule *replayer = NULL;
	ReplayStats *stats = NULL;

	/* Option Processing */
	boost::program_options::options_description visible("Allowed options");
	visible.add_options()
		("help,h", "produce help message")
		("verbose,v", "replay in verbose mode")
		("mode,m", boost::program_options::value<std::string>(),
					"-sync or -async replaying mode")
		("device,d", boost::program_options::value<std::string>(),
					"device_id")
		("config,c", boost::program_options::value<std::string>(),
					"config file")
	;

	boost::program_options::options_description desc("Hidden options");
	desc.add_options()
		("input-files", boost::program_options::
					value<std::vector<std::string> >(),
					"input files")
		;
	desc.add(visible);

	boost::program_options::positional_options_description p;
	p.add("input-files", -1);

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::
					command_line_parser(argc, argv).
					options(desc).positional(p).run(), vm);
	boost::program_options::notify(vm);

	if (vm.count("help")) {
		std::cerr << visible << "\n";
		goto cleanup;
	}

	if (vm.count("verbose"))
		verbose = true;

	if (vm.count("mode")) {
		mode = vm["mode"].as<std::string>();
		if (mode != "sync" and mode != "async") {
			std::cerr << "Unsupported mode: " << mode << ".\n";
			goto cleanup;
		}
	} else {
		std::cerr << "No mode specified.\n";
		goto cleanup;
	}

	if (vm.count("device"))
		device_id = vm["device"].as<std::string>();
	else {
		std::cerr << "No device specified.\n";
		goto cleanup;
	}

	if (vm.count("config"))
		config_file = vm["config"].as<std::string>();
	else {
		std::cerr << "No configuration file specified.\n";
		goto cleanup;
	}

	if (vm.count("input-files")) {
		input_files = vm["input-files"].as<std::vector<std::string> >();
	} else {
		std::cout << "No input dataseries files.\n";
		goto cleanup;
	}

	/* create a Commander that executes operations and create buffer
		alligned with page by floor division */
	mbuffer = malloc(BUFFER_SIZE);
	if (!mbuffer) {
		std::cerr << "Out of memory. Unable to allocate buffer of size " <<
				BUFFER_SIZE << " bytes.\n";
		goto cleanup;
	}

	/* buffer represents mbuffer aligned to a PAGE_SIZE'd boundary. */
	buffer = (void*)(((uint64_t)(mbuffer) + PAGE_SIZE - 1) / PAGE_SIZE
			* PAGE_SIZE);

	stats = new ReplayStats;
	if (mode == "sync")
		commander = new SynchronousCommander(device_id,
						     buffer,
						     verbose,
						     stats);
	else
		commander = new AsynchronousCommander(device_id,
						      buffer,
						      verbose,
						      stats);

	/* Read in the configuration file */
	input.open(config_file.c_str());

	rowNum = 0;
	while (getline(input, line)) {
		rowNum++;
		std::vector<std::string> split_data;
		boost::split(split_data, line, boost::is_any_of(","));

		std::vector<uint64_t> configuration_data;
		for (std::vector<std::string>::iterator
						iter = ++split_data.begin();
						iter != split_data.end();
						iter++)
			configuration_data.push_back((uint64_t)atoll(iter->c_str()));

		if (configuration_data.size() != 4) {
			std::cerr << "Illegal row in configuration file: " << rowNum <<
					": '" << line << "'.\n";
			goto cleanup;
		}

		configuration[split_data[0]] = configuration_data;
	}

	if (configuration.size() == 0) {
		std::cerr << "Invalid configuration file.\n";
		goto cleanup;
	}

	input.close();

	/* Specify to read in the extent type "read_write" */
	for (std::vector<std::string>::iterator iter = input_files.begin();
				      iter != input_files.end();
				      iter++)
		source.addSource(*iter);

	/* Set up the interrupt based status report mechanism. */
	rs = stats;
	struct sigaction usr1_action;
	usr1_action.sa_handler = reportProgress;
	sigemptyset(&usr1_action.sa_mask);
	usr1_action.sa_flags = 0;
	sigaction (SIGUSR1, &usr1_action, NULL);

	/* Parallel decompress and stats, 64MiB buffer */
	prefetch = new PrefetchBufferModule(source, 64 * 1024 * 1024);
	replayer = new BlocktraceReplayModule(*prefetch, configuration, commander,
			stats, verbose);

	/* Replay all extents. */
	while (replayer->getExtent());

	replayer->printResult();

	/* All is well */
	ret = EXIT_SUCCESS;

cleanup:
	if (mbuffer)
		free(mbuffer);

	if (stats) {
		rs = NULL;
		delete stats;
	}

	if (commander) {
		delete commander;
	}

	if (prefetch)
		delete prefetch;

	if (replayer)
		delete replayer;

	return ret;
}
