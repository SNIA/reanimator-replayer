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

/*
 * A program to write a single row into a DataSeries file
 * with a single extent type. Extent's schema is:
 * timestamp, filename, opcode, offset, iosize
 */

#include <iostream>

#include <DataSeries/ExtentType.hpp>
#include <DataSeries/DataSeriesFile.hpp>
#include <DataSeries/DataSeriesModule.hpp>

int main(int argc, char *argv[]) {
	if (argc != 7) {
		std::cout << "Wrong usage!\n";
		std::cout << "Usage: ds-simple-write <timestamp> <filename> "
				"<opcode> <offset> <iosize> <outfile>\n";
		std::cout << "Field types:\n";
		std::cout << "\ttimestamp - int64\n";
		std::cout << "\tfilename - variable32 (string)\n";
		std::cout << "\topcode - byte (\"r\" or \"w\")\n";
		std::cout << "\toffset - int64\n";
		std::cout << "\tiosize - int64\n";
		return 1;
	}

	const char *outfile = argv[6];

	/*
	 * First, let'a define a schema for our extent.  Don't forget to
	 * include namespace and version information: you will get a warning if
	 * you don't have either a namespace or a version.  Remember to follow
	 * the heirarchical naming conventions when naming extents.  It's mostly
	 * like Trace::<type of trace>::<Machine type>::
	 * <workload type>::<anything else that is appropriate>
	 */
	const char *extentXMLDescription =
		"<ExtentType namespace=\"http://www.fsl.cs.sunysb.edu/\""
		  " name=\"Trace::Fictitious::Linux\""
		  " version=\"0.1\""
		  " comment=\"extent comment does NOT go to the ds file\">\n"
		"  <field type=\"int64\" name=\"timestamp\""
		  " comment=\"field comment does NOT go to the ds file\"/>\n"
		"  <field type=\"variable32\" name=\"filename\"/>\n"
		"  <field type=\"byte\" name=\"opcode\"/>\n"
		"  <field type=\"int64\" name=\"offset\"/>\n"
		"  <field type=\"int64\" name=\"iosize\"/>\n"
		"</ExtentType>\n";

	ExtentTypeLibrary extentTypeLibrary;

	const ExtentType &extentType =
		 extentTypeLibrary.registerTypeR(extentXMLDescription);

	/*
	 * Sink is a wrapper for a DataSeries output file.
	 * Create a sink and write out the extent type extent.
	 */
	DataSeriesSink outfileSink(outfile);
	outfileSink.writeExtentLibrary(extentTypeLibrary);

	/*
	 * Now create an output module that
	 * processes extents using ExtentSeries (iterator)
	 * and put them into the sink.
	 */
	uint32_t target_extent_size = 4096;
	ExtentSeries extentSeries;
	OutputModule outputModule(outfileSink, extentSeries,
					extentType, target_extent_size);

	/*
	 * These are handles for the fields in the
	 * "current record" of extentSeries.
	 */
	Int64Field timestamp(extentSeries, "timestamp");
	Variable32Field filename(extentSeries, "filename");
	ByteField opcode(extentSeries, "opcode");
	Int64Field offset(extentSeries, "offset");
	Int64Field size(extentSeries, "iosize");

	/*
	 * Initiate a new record in extentSeries
	 */
	outputModule.newRecord();

	/*
	 * Set new records of the extentSeries
	 */
	timestamp.set((ExtentType::int64)atoi(argv[1]));
	filename.set(argv[2]);
	if (!strcmp(argv[3], "r"))
		opcode.set((ExtentType::byte)0x0);
	else if (!strcmp(argv[3], "w"))
		opcode.set((ExtentType::byte)0x1);
	else
		opcode.set((ExtentType::byte)0x2);
	offset.set((ExtentType::int64)atoi(argv[4]));
	size.set((ExtentType::int64)atoi(argv[5]));

	/*
	 * Ask output module to finilize the file
	 */
	outputModule.flushExtent();
	outputModule.close();

	return 0;
}
