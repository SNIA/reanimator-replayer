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
 * This program reads DataSeries files created by ds-simple-write.C
 * program and prints each row contained in the files, as well
 * as the total number of rows.
 *
 * To do this, we create a class that derives from RowAnalysisModule.
 * RowAnalysisModule is a built-in module that operates over every row,
 * one at a time. We override RowAnalysisModule virtual functions
 * prepareForProcessing(), processRow(), and completeProcessing()
 * to do specific analysis:
 *
 * prepareForProcessing() method is called once before
 * the first row is processed.
 * processRow() is called for each row.
 * completeProcessing() is called after the last row is processed.
 *
 * In this case, our analysis is very simple:
 * - print each row
 * - and count the number of rows
 *
 */

#include <iostream>

#include <DataSeries/RowAnalysisModule.hpp>
#include <DataSeries/DataSeriesModule.hpp>
#include <DataSeries/ExtentField.hpp>
#include <DataSeries/TypeIndexModule.hpp>

class PrintRowsModule : public RowAnalysisModule {
private:
	int count_;
	Int64Field timestamp_;
	Variable32Field filename_;
	ByteField opcode_;
	Int64Field offset_;
	Int64Field iosize_;

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
		timestamp_(series, "timestamp"),
		filename_(series, "filename"),
		opcode_(series, "opcode"),
		offset_(series, "offset"),
		iosize_(series, "iosize"),
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
		std::cout << count_ << "\t";
		std::cout << timestamp_.val() << "\t";
		std::cout << filename_.val() << "\t";
		if (opcode_.val() == 0x0)
			std::cout << "r\t";
		else if (opcode_.val() == 0x1)
			std::cout << "r\t";
		else
			std::cout << "unkn\t";
		std::cout << offset_.val() << "\t";
		std::cout << iosize_.val() << "\n";
		count_++;
	}

	/* This function will be called once by RowAnalysisModule after all
	 * data (extents and rows) are processed. */
	void completeProcessing()
	{
		std::cout << count_ << " row(s) processed\n";
	}
};

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cout << "Too few parameters!\n";
		std::cout << "Usage: ds-simple-read <filename> ...\n";
		return 1;
	}

	/*
	 * The first thing to do is to specify the type of
	 * extents that are going to be processed.
	 * TypeIndexModule class reads all extents of a
	 * specified type from a set of DataSeries files.
	 */
	TypeIndexModule source("Trace::Fictitious::Linux");

	for (int i = 1; i < argc; i++)
		source.addSource(argv[i]);

	/* Now we create our module based on the source */
	PrintRowsModule processor(source);

	/* getAndDelete() method initiates iteration over all rows */
	processor.getAndDelete();
}
