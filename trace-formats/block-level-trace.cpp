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
#include <fstream>

#include <DataSeries/ExtentType.hpp>
#include <DataSeries/DataSeriesFile.hpp>
#include <DataSeries/DataSeriesModule.hpp>

#define SYSTEM_ID_VM_BLOCK	10
#define SYSTEM_ID_GPFS_BLOCK	20
#define DEVICE_ID_VM_SDA	10
#define DEVICE_ID_GPFS_SDB	20
#define PROCESS_ID_VM_PDFLUSH	123
#define PROCESS_ID_GPFS_PDFLUSH	456
#define SECOND ((int64_t)1 << 32)

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cout << "Wrong usage!\n";
		std::cout << "Usage: " << argv[0] << " <filename>\n";
		return 1;
	}

	const char *outfile = argv[1];

	/* Loading extent XML descriptions */
	std::ifstream commentExtentXMLFile("comment-extent.xml");
	if (!commentExtentXMLFile.is_open()) {
		std::cout << "Coud not open comment-extent.xml file!\n";
		return 1;
	}

	char commentExtentXMLDescription[4096];
	commentExtentXMLFile.read(commentExtentXMLDescription, 4096);

	std::ifstream mdExtentXMLFile("block_metadata-extent.xml");
	if (!mdExtentXMLFile.is_open()) {
		std::cout << "Coud not open block_metadata-extent.xml file!\n";
		return 1;
	}

	char mdExtentXMLDescription[4096];
	mdExtentXMLFile.read(mdExtentXMLDescription, 4096);

	std::ifstream rwExtentXMLFile("read_write-extent.xml");
	if (!rwExtentXMLFile.is_open()) {
		std::cout << "Coud not open read_write-extent.xml file!\n";
		return 1;
	}

	char rwExtentXMLDescription[4096];
	rwExtentXMLFile.read(rwExtentXMLDescription, 4096);

	/* Registering extent types to the library */
	ExtentTypeLibrary extentTypeLibrary;

	const ExtentType &rwExtentType =
		 extentTypeLibrary.registerTypeR(rwExtentXMLDescription);

	const ExtentType &commentExtentType =
		 extentTypeLibrary.registerTypeR(commentExtentXMLDescription);

	const ExtentType &mdExtentType =
		 extentTypeLibrary.registerTypeR(mdExtentXMLDescription);

	/*
	 * Sink is a wrapper for a DataSeries output file.
	 * Create a sink and write out the extent type extent.
	 */
	DataSeriesSink outfileSink(outfile);
	outfileSink.writeExtentLibrary(extentTypeLibrary);

	uint32_t target_extent_size = 4096;

	/* Populating comments extents */
	ExtentSeries commentExtentSeries;
	OutputModule commentOutputModule(outfileSink, commentExtentSeries,
					commentExtentType, target_extent_size);
	Variable32Field comment(commentExtentSeries, "comments");
	commentOutputModule.newRecord();
	comment.set("Time units are tfracs (2^-32 of a seconds), absolute"
			" starting from UNIX epoch.");
	commentOutputModule.flushExtent();
	commentOutputModule.close();

	/* Populating block_metadata extents */
	ExtentSeries mdExtentSeries;
	OutputModule mdOutputModule(outfileSink, mdExtentSeries,
					mdExtentType, target_extent_size);
	Variable32Field type(mdExtentSeries, "type");
	Int32Field identifier(mdExtentSeries, "identifier");
	Variable32Field description(mdExtentSeries, "description");
	mdOutputModule.newRecord();
	type.set("system");
	description.set("Block layer in VM");
	identifier.set(SYSTEM_ID_VM_BLOCK);
	mdOutputModule.newRecord();
	type.set("system");
	description.set("Block layer in GPFS");
	identifier.set(SYSTEM_ID_GPFS_BLOCK);
	mdOutputModule.newRecord();
	type.set("device");
	description.set("/dev/sda");
	identifier.set(DEVICE_ID_VM_SDA);
	mdOutputModule.newRecord();
	type.set("device");
	description.set("/dev/sdb");
	identifier.set(DEVICE_ID_GPFS_SDB);
	mdOutputModule.newRecord();
	type.set("process");
	description.set("pdflush in VM");
	identifier.set(PROCESS_ID_VM_PDFLUSH);
	mdOutputModule.newRecord();
	type.set("process");
	description.set("pdflush at gpfs server");
	identifier.set(PROCESS_ID_GPFS_PDFLUSH);
	mdOutputModule.flushExtent();
	mdOutputModule.close();

	/* Populating read_write extents */
	ExtentSeries rwExtentSeries;
	OutputModule rwOutputModule(outfileSink, rwExtentSeries,
					rwExtentType, target_extent_size);
	Int64Field logical_IO_id(rwExtentSeries, "logical_IO_id", 1);
	Int64Field enter_time(rwExtentSeries, "enter_time", 1);
	Int64Field leave_time(rwExtentSeries, "leave_time", 1);
	Int64Field return_time(rwExtentSeries, "return_time", 1);
	Int64Field exit_time(rwExtentSeries, "exit_time", 1);
	Int32Field system_id(rwExtentSeries, "system_id", 1);
	Int32Field process_id(rwExtentSeries, "process_id", 1);
	Int32Field device_id(rwExtentSeries, "device_id");
	Int32Field completion_status(rwExtentSeries, "completion_status", 1);
	ByteField operation(rwExtentSeries, "operation");
	Int64Field offset(rwExtentSeries, "offset");
	Int64Field request_size(rwExtentSeries, "request_size");
	Int64Field actual_size(rwExtentSeries, "actual_size", 1);
	ByteField semantic_type(rwExtentSeries, "semantic_type", 1);
	Variable32Field semantic_type_name(rwExtentSeries,
					"semantic_type_name", 1);

	rwOutputModule.newRecord();
	logical_IO_id.setNull();
	enter_time.set(1312397100 * SECOND);
	leave_time.set(1312397101 * SECOND);
	return_time.set(1312397106 * SECOND);
	exit_time.set(1312397107 * SECOND);
	system_id.set(SYSTEM_ID_VM_BLOCK);
	process_id.set(PROCESS_ID_VM_PDFLUSH);
	device_id.set(DEVICE_ID_VM_SDA);
	operation.set((ExtentType::byte)0x1); /* write */
	offset.set(8192);
	request_size.set(4096);
	actual_size.setNull();
	semantic_type.setNull();
	semantic_type_name.setNull();

	rwOutputModule.newRecord();
	logical_IO_id.setNull();
	enter_time.set(1312397102 * SECOND);
	leave_time.set(1312397103 * SECOND);
	return_time.set(1312397104 * SECOND);
	exit_time.set(1312397105 * SECOND);
	system_id.set(SYSTEM_ID_GPFS_BLOCK);
	process_id.set(PROCESS_ID_GPFS_PDFLUSH);
	device_id.set(DEVICE_ID_GPFS_SDB);
	operation.set((ExtentType::byte)0x1); /* write */
	offset.set(32768);
	request_size.set(4096);
	actual_size.setNull();
	semantic_type.setNull();
	semantic_type_name.setNull();

	rwOutputModule.flushExtent();
	rwOutputModule.close();

	return 0;
}
