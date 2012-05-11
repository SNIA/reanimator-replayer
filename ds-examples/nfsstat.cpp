/*
 * Copyright (c) 2012 Sudhir Kasanavesi
 * Copyright (c) 2012 Kalyan Chandra
 * Copyright (c) 2012 Nihar Reddy
 * Copyright (c) 2012 Vasily Tarasov
 * Copyright (c) 2012 Erez Zadok
 * Copyright (c) 2012 Stony Brook University
 * Copyright (c) 2012 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is a small analyzer module which does some basic analysis on the
 * DataSeries file produced by nfstrace2ds.sh.
 */

#include <iostream>
#include <list>

#include <DataSeries/RowAnalysisModule.hpp>
#include <DataSeries/DataSeriesModule.hpp>
#include <DataSeries/ExtentField.hpp>
#include <DataSeries/TypeIndexModule.hpp>

using namespace std;

class CountRowsModule : public RowAnalysisModule {
private:
	uint64_t count_;
	Int64Field time_called_;
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
	CountRowsModule(DataSeriesModule &source) :
		RowAnalysisModule(source),
		count_(0),
		time_called_(series, "time_called")
		{}

	void prepareForProcessing()
	{
	}

	/* This function will be called by RowAnalysisModule once
	 * for every row in the Extents being processed. */
	void processRow()
	{
		count_++;
	}

	/* This function will be called once by RowAnalysisModule after all
	 * data (extents and rows) are processed. */
	void completeProcessing()
	{
	}
	
	/* This function can be called to extract the count of extent type records
	 */
	uint64_t getCount()
	{
		return count_;
	}
};


class PrintRowsModule : public RowAnalysisModule {
private:
	uint64_t count_;
	Int64Field time_called_;
	Int32Field xid_;
	Variable32Field file_handle_;
	Int64Field offset_;
	Int64Field length_;
	list<uint64_t> L;
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
		count_(0),
		time_called_(series, "time_called"),
		xid_(series, "xid"),
		file_handle_(series, "file_handle"),
		offset_(series, "offset"),
		length_(series, "length")
		{}

	void prepareForProcessing()
	{
		/*cout << "row\ttime_called\txid\tfile_handle\toffset\t"
					 << "length\n";
		*/
	}

	/* This function will be called by RowAnalysisModule once
	 * for every row in the Extents being processed. */
	void processRow()
	{
		/*cout << count_ << "\t";
		cout << time_called_.val() << "\t";
		cout << xid_.val() << "\t";
		cout << file_handle_.val() << "\t";
		cout << offset_.val() << "\t";
		cout << length_.val() << "\n";
		 */
		L.push_back((uint64_t)length_.val());
		count_++;
	}

	/* This function will be called once by RowAnalysisModule after all
	 * data (extents and rows) are processed. */
	void completeProcessing()
	{
		//cout << count_ << " row(s) processed\n";
	}
	
	void displayReadWriteStatistics()
	{
		uint64_t sum = 0;
		double mean =0.0;
		list<uint64_t>::iterator i;
		for(i=L.begin(); i != L.end(); ++i)
		{
			sum = sum + *i;
		}
		mean=((double)sum/(double)count_);
		cout << "Total I/O Size: "  << sum <<endl ;
		cout << "Mean I/O Size: " << mean <<endl;
		L.sort();

		list<uint64_t>::iterator iii;
		double tempvariance = 0.0, variance = 0.0, stddeviation = 0.0;
		for(iii=L.begin(); iii != L.end(); ++iii)
		{                       
			tempvariance = tempvariance + (double)(((double)*iii - mean)*((double)*iii - mean));
		}
		variance = (double)(tempvariance/(double)count_);
		stddeviation = sqrt(variance);
		cout << "Std Deviation: " << stddeviation << endl;
	}
};

int main(int argc, char *argv[])
{
	if (argc < 2) {
		cout << "Too few parameters!\n";
		cout << "Usage: nfsstat <filename> \n";
		return 1;
	}

	/*
	 * The first thing to do is to specify the type of
	 * extents that are going to be processed.
	 * TypeIndexModule class reads all extents of a
	 * specified type from a set of DataSeries files.
	 */
	TypeIndexModule writes("IOTTAFSL:Trace:Syscall:write_request");
	TypeIndexModule reads("IOTTAFSL:Trace:Syscall:read_request");
	TypeIndexModule lookups("IOTTAFSL:Trace:Syscall:lookup_request");
	TypeIndexModule accesss("IOTTAFSL:Trace:Syscall:access_request");
	TypeIndexModule creates("IOTTAFSL:Trace:Syscall:create_request");
	TypeIndexModule fsinfos("IOTTAFSL:Trace:Syscall:fsinfo_request");
	TypeIndexModule pathconfs("IOTTAFSL:Trace:Syscall:pathconf_request");
	TypeIndexModule mkdirs("IOTTAFSL:Trace:Syscall:mkdir_request");
	TypeIndexModule removes("IOTTAFSL:Trace:Syscall:remove_request");
	TypeIndexModule getattrs("IOTTAFSL:Trace:Syscall:getattr_request");
	TypeIndexModule commits("IOTTAFSL:Trace:Syscall:commit_request");
	TypeIndexModule writes1("IOTTAFSL:Trace:Syscall:write_request");
	TypeIndexModule reads1("IOTTAFSL:Trace:Syscall:read_request");
	
	writes.addSource(argv[1]);
	reads.addSource(argv[1]);
	lookups.addSource(argv[1]);		
	accesss.addSource(argv[1]);
	creates.addSource(argv[1]);
	fsinfos.addSource(argv[1]);
	pathconfs.addSource(argv[1]);
	mkdirs.addSource(argv[1]);
	removes.addSource(argv[1]);
	getattrs.addSource(argv[1]);
	commits.addSource(argv[1]);
	reads1.addSource(argv[1]);
	writes1.addSource(argv[1]);

	/* Now we create our module based on the source */
	CountRowsModule writeProcessor(writes);
	CountRowsModule readProcessor(reads);
	CountRowsModule lookupProcessor(lookups);
	CountRowsModule accessProcessor(accesss);
	CountRowsModule createProcessor(creates);
	CountRowsModule fsinfoProcessor(fsinfos);
	CountRowsModule pathconfProcessor(pathconfs);
	CountRowsModule mkdirProcessor(mkdirs);
	CountRowsModule removeProcessor(removes);
	CountRowsModule getattrProcessor(getattrs);
	CountRowsModule commitProcessor(commits);
	PrintRowsModule readPrinter(reads1);
	PrintRowsModule writePrinter(writes1);

	/* getAndDelete() method initiates iteration over all rows */
	readProcessor.getAndDelete();
	writeProcessor.getAndDelete();
	lookupProcessor.getAndDelete();
	accessProcessor.getAndDelete();
	createProcessor.getAndDelete();
	fsinfoProcessor.getAndDelete();
	pathconfProcessor.getAndDelete();
	mkdirProcessor.getAndDelete();
	removeProcessor.getAndDelete();
	getattrProcessor.getAndDelete();
	commitProcessor.getAndDelete();
	
	cout<< "Procedure\tCount"<<endl;
	cout<< "READ\t\t" << readProcessor.getCount()<<endl;
	cout<< "WRITE\t\t" << writeProcessor.getCount()<<endl;
	cout<< "LOOKUP\t\t" << lookupProcessor.getCount()<<endl;
	cout<< "ACCESS\t\t" << accessProcessor.getCount()<<endl;
	cout<< "CREATE\t\t" << createProcessor.getCount()<<endl;
	cout<< "FSINFO\t\t" << fsinfoProcessor.getCount()<<endl;
	cout<< "PATHCONF\t" << pathconfProcessor.getCount()<<endl;
	cout<< "MKDIR\t\t" << mkdirProcessor.getCount()<<endl;
	cout<< "REMOVE\t\t" << removeProcessor.getCount()<<endl;
	cout<< "GETATTR\t\t" << getattrProcessor.getCount()<<endl;
	cout<< "COMMIT\t\t" << commitProcessor.getCount()<<endl;
	
	readPrinter.getAndDelete();
	writePrinter.getAndDelete();
	cout<< endl << "READ Statistics " << endl;
	readPrinter.displayReadWriteStatistics();
	cout<< endl << "WRITE Statistics " << endl;
	writePrinter.displayReadWriteStatistics();
	cout<<endl;
}
