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

/*
 * This program reads DataSeries files and extrace the write operations in the
 * SNIA animation NFS traces at http://iotta.snia.org/tracetypes/2.
 *
 * NFS Trace format (as output head of ds2txt):
 *
 * <ExtentType namespace="ssd.hpl.hp.com" name="Trace::NFS::read-write"
 * version="1.0" >
 * <field type="int64" name="request_id" comment="for correlating with the
 *      records in other extent types" pack_relative="request-id" />
 * <field type="int64" name="reply_id" comment="for correlating with the
 *      records in other extent types" pack_relative="request-id" />
 * <field type="variable32" name="filehandle" print_style="hex"
 *	pack_unique="yes" />
 * <field type="bool" name="is_read" />
 * <field type="int64" name="offset" />
 * <field type="int32" name="bytes" />
 * </ExtentType>
 *
 *
 * Example usage:
 *  
 *	$ ./extract-snia-nfs-write ~/Downloads/nfs-ds/set2-000000-000499.ds
 */

#include <iostream>
#include <functional>

#include <DataSeries/RowAnalysisModule.hpp>
#include <DataSeries/DataSeriesModule.hpp>
#include <DataSeries/ExtentField.hpp>
#include <DataSeries/TypeIndexModule.hpp>

class PrintRowsModule : public RowAnalysisModule {
private:
	int count_;
	Int64Field request_id_;
	Int64Field reply_id_;
	Variable32Field filehandle_;
	BoolField is_read_;
	Int64Field offset_;
	Int32Field bytes_;

	std::hash<std::string> hasher_;

public:
	/*
	 * Our constructor takes DataSeriesModule from which to get
	 * extents and passes it on to the base class constructor.
	 *
	 * We use the inherited ExtentSeries 'series' to
	 * initialize the Fields. An ExtentSeries is an iterator
	 * over the records in extents. Fields are connected to
	 * a particular ExtentSeries and provide access to the
	 * values of each record as the ExtentSeries points to it.
	 * our fields are:
	 */
	PrintRowsModule(DataSeriesModule &source) :
		RowAnalysisModule(source),
		request_id_(series, "request_id"),
		reply_id_(series, "reply_id"),
		filehandle_(series, "filehandle"),
		is_read_(series, "is_read"),
		offset_(series, "offset"),
		bytes_(series, "bytes"),
		count_(0) {}

	void prepareForProcessing()
	{
		std::cout << "row\ttstmp\tfname\topcode\t"
					 << "offset\tiosize\n";
	}

	/* This function will be called by RowAnalysisModule once
	 * for every row in the Extents being processed. */
	void processRow()
	{
		if (!is_read_.val()) {
			std::cout << hasher_(filehandle_.stringval()) << "\t"
				<< offset_.val() << "\t"
				<< bytes_.val() << "\n";
			count_++;
		}
	}

	/* This function will be called once by RowAnalysisModule after all
	 * data (extents and rows) are processed. */
	void completeProcessing()
	{
		std::cout << count_ << " write(s) processed\n";
	}
};

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cout << "Too few parameters!\n";
		std::cout << "Usage: extract-snia-nfs-write <filename> ...\n";
		return 1;
	}

	/*
	 * The first thing to do is to specify the type of
	 * extents that are going to be processed.
	 * TypeIndexModule class reads all extents of a
	 * specified type from a set of DataSeries files.
	 */
	TypeIndexModule source("Trace::NFS::read-write");

	for (int i = 1; i < argc; i++)
		source.addSource(argv[i]);

	/* Now we create our module based on the source */
	PrintRowsModule processor(source);

	/* getAndDelete() method initiates iteration over all rows */
	processor.getAndDelete();
}
