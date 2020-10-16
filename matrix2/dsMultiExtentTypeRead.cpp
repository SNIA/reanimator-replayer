/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
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
#include <set>

#include <DataSeries/PrefetchBufferModule.hpp>
#include <DataSeries/TypeIndexModule.hpp>
#include <DataSeries/RowAnalysisModule.hpp>
#include <Lintel/LintelLog.hpp>

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <cstring>
#include <csignal>

#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>

#include <stdexcept>

int main(int argc, char *argv[])
{
	int ret = 0;
	std::string traceFileName = argv[1];

/*	
 *	Enable, if you want debugging:
 *
 *	LintelLog::setDebugLevel("ExtentType::GetColNum", 10);
 *	LintelLog::setDebugLevel("*", 10);
 *
 */
	
	TypeIndexModule source_write("IOTTAFSL:Trace:Syscall:write_request");
	TypeIndexModule source_read("IOTTAFSL:Trace:Syscall:read_request");

	source_write.addSource(traceFileName);
	source_read.addSource(traceFileName);

	PrefetchBufferModule prefetch_write(source_write, (64 * 1024 * 1024));
	PrefetchBufferModule prefetch_read(source_read, (64 * 1024 * 1024));

	Extent::Ptr current_extent_write = prefetch_write.getSharedExtent();
	Extent::Ptr current_extent_read = prefetch_read.getSharedExtent();

	ExtentSeries current_series_write = ExtentSeries(current_extent_write);
	ExtentSeries current_series_read = ExtentSeries(current_extent_read);

	Int64Field offset_write(current_series_write, "offset");
	Int64Field time_called_write(current_series_write, "time_called");
	Variable32Field fhandle_write(current_series_write, "file_handle");
	Int64Field length_write(current_series_write, "length");

	Int64Field offset_read(current_series_read, "offset");
	Int64Field time_called_read(current_series_read, "time_called");
	Variable32Field fhandle_read(current_series_read, "file_handle");
	Int64Field length_read(current_series_read, "length");

	while (1) {
		if (!current_extent_write && !current_extent_read)
			break;

		/* We ran out of write extents */	
		if (!current_extent_write && current_extent_read) {
			if (current_series_read.morerecords()) {
				std::cout << "read_request,"
					  << time_called_read.val() << ","
					  << fhandle_read.stringval() << ","
					  << offset_read.val() << ","
					  << length_read.val() << ",\n";

				++current_series_read;
			} else {
				current_extent_read = prefetch_read.getSharedExtent();
				if (current_extent_read)
					current_series_read.setExtent(current_extent_read);
				/* next iteration will pick up next record */
			}
			continue;
		}

		/* We ran out of read extents */	
		if (!current_extent_read && current_extent_write) {
			if (current_series_write.morerecords()) {
			std::cout << "write_request,"
				  << time_called_write.val() << ","
				  << fhandle_write.stringval() << ","
				  << offset_write.val() << ","
				  << length_write.val() << ",,,,\n";

				++current_series_write;
			} else  {
				current_extent_write = prefetch_write.getSharedExtent();
				if (current_extent_write)
					current_series_write.setExtent(current_extent_write);
				/* next iteration will pick up next record */
			}
			continue;
		}

		/* We ran out of write records in current extent */	
		if (!current_series_write.morerecords()) {
			current_extent_write = prefetch_write.getSharedExtent();
			if (current_extent_write)
				current_series_write.setExtent(current_extent_write);
			/* next iteration will pick up next record */
			continue;
		}

		/* We ran out of read records in current extent */	
		if (!current_series_read.morerecords()) {
			current_extent_read = prefetch_read.getSharedExtent();
			if (current_extent_read)
				current_series_read.setExtent(current_extent_read);
			/* next iteration will pick up next record */
			continue;
		}

		/* We have records of both type: reads and writes */	
		if (time_called_write.val() <= time_called_read.val()) {
			std::cout << "write_request,"
				  << time_called_write.val() << ","
				  << fhandle_write.stringval() << ","
				  << offset_write.val() << ","
				  << length_write.val() << ",,,,\n";
			++current_series_write;
		} else {
			std::cout << "read_request,"
				  << time_called_read.val() << ","
				  << fhandle_read.stringval() << ","
				  << offset_read.val() << ","
				  << length_read.val() << ",\n";
			++current_series_read;
		}
	}

	return ret;
}
