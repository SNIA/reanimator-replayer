/*
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

extern "C" {
#include "feature_matrix.h"
}

t2mfm *fmh = NULL;

static int insert_data(const t2mfm_vec *p_dim_data_vec,
		uint8_t procedure, uint64_t offset, uint64_t length)
{
	int rc;
	int op;

	assert(fmh);
	assert(p_dim_data_vec && (p_dim_data_vec->n_elements == 3));

	T2MFM_SET_DIM_DATA(*p_dim_data_vec, 0, INT1, procedure);
	T2MFM_SET_DIM_DATA(*p_dim_data_vec, 1, INT8, offset);
	T2MFM_SET_DIM_DATA(*p_dim_data_vec, 2, INT8, length);

	rc = t2mfm_insert(fmh);
	T2MFM_HANDLE_RC(fmh, rc, cleanup);

cleanup:

	if (rc)
		exit(1);
	
	return rc;
}


static int add_dimensions()
{
	int rc;
	/* Declare new dimensions, statically initializing their meta-data */
	t2mfm_dim_meta opcode = {0, "procedure",
			"procedures: 0 - read, 1 - write", T2MFM_INT1,
			T2MFM_TRUE};
	t2mfm_dim_meta offset = {1, "offset", "offset in bytes", T2MFM_INT8,
			T2MFM_TRUE};
	t2mfm_dim_meta length = {2, "length", "length in bytes", T2MFM_INT8,
			T2MFM_TRUE};

	assert(fmh);

	/* Begin adding dimensions */
	rc = t2mfm_add_dim_meta_begin(fmh);
	T2MFM_HANDLE_RC(fmh, rc, cleanup);

	/* Add individual dimensions */
	rc = t2mfm_add_dim_meta(fmh, &opcode);
	T2MFM_HANDLE_RC(fmh, rc, cleanup);

	rc = t2mfm_add_dim_meta(fmh, &offset);
	T2MFM_HANDLE_RC(fmh, rc, cleanup);

	rc = t2mfm_add_dim_meta(fmh, &length);
	T2MFM_HANDLE_RC(fmh, rc, cleanup);

cleanup:
	t2mfm_add_dim_meta_end(fmh); /* XXX: No error handling */

	if (rc)
		exit(1);

	return rc;
}


void init_matrix(char *matrix_name, char *store_file_name)
{
	int ret;

	ret = t2mfm_open(store_file_name, matrix_name, T2MFM_OPEN_CREATE, &fmh);
	if (ret)
		exit(1);
}

int main(int argc, char *argv[])
{
	int ret = 0;
	std::string traceFileName = argv[1];
	char *store_file_name = argv[2];
	char matrix_name[] = "t2mmatrix";
	t2mfm_vec dim_data_vec;

	init_matrix(matrix_name, store_file_name);

	add_dimensions();

	ret = t2mfm_insert_begin(fmh, &dim_data_vec);
	if (ret)
		exit(1);

	/* We expect dimensions procedure, offset and length */
	assert(dim_data_vec.n_elements == 3);

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
				insert_data(&dim_data_vec, 0, offset_read.val(), length_read.val());
				/*
				std::cout << "read_request,"
					  << time_called_read.val() << ","
					  << fhandle_read.stringval() << ","
					  << offset_read.val() << ","
					  << length_read.val() << ",\n";
				*/

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
				insert_data(&dim_data_vec, 1, offset_write.val(), length_write.val());
				/*
				std::cout << "write_request,"
				 	  << time_called_write.val() << ","
					  << fhandle_write.stringval() << ","
					  << offset_write.val() << ","
					  << length_write.val() << ",,,,\n";
				*/

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
			insert_data(&dim_data_vec, 1, offset_write.val(), length_write.val());
			/*
			std::cout << "write_request,"
				  << time_called_write.val() << ","
				  << fhandle_write.stringval() << ","
				  << offset_write.val() << ","
				  << length_write.val() << ",,,,\n";
			*/
			++current_series_write;
		} else {
			insert_data(&dim_data_vec, 0, offset_read.val(), length_read.val());
			/*
			std::cout << "read_request,"
				  << time_called_read.val() << ","
				  << fhandle_read.stringval() << ","
				  << offset_read.val() << ","
				  << length_read.val() << ",\n";
			*/
			++current_series_read;
		}
	}


	t2mfm_insert_end(fmh); /* XXX: Don't handle errors here or we'll recurse */


	return ret;
}
