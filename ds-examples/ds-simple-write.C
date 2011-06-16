/*
 * A program to write out a single row into a DataSeries
 * file that has a single extent type with schema:
 * timestamp<int64>, op_code<byte>, size<int64>, filename<variable length string>
 */

#include <iostream>

#include <DataSeries/ExtentType.hpp>
#include <DataSeries/DataSeriesFile.hpp>
#include <DataSeries/DataSeriesModule.hpp>

using namespace std;

int main(int argc, char *argv[]) {
	if (argc < 6) {
		cout << "Too few parameters!\n";
		cout << "Usage: dswrite <timestamp> <op_code> <size>"
				" <filename> <outfile>\n";
		cout << "Field types:\n";
		cout << "\t timestamp - int64\n";
		cout << "\t op_code - byte\n";
		cout << "\t size - int64\n";
		cout << "\t filename - variable32 (string)\n";
		return 1;
	}

	/*
	 * First, let'a define a schema for our extent.  Don't forget to
	 * include namespace and version information: you will get a warning if
	 * you don't have either a namespace or a version.  Remember to follow
	 * the heirarchical naming conventions when naming extents.  It's mostly
	 * like Trace::<type of trace>::<Machine type>::<workload
	 * type>::<anything else that is appropriate>
	 */
	const char *extentXMLDescription =
		"<ExtentType name=\"Trace::Fictitious::Linux\""
			" namespace=\"http://www.fsl.cs.sunysb.edu/\""
			" version=\"0.1\">\n"
		"  <field type=\"int64\" name=\"timestamp\"/>\n"
		"  <field type=\"byte\" name=\"op_code\"/>\n"
		"  <field type=\"int64\" name=\"size\"/>\n"
		"  <field type=\"variable32\" name=\"filename\"/>\n"
		"</ExtentType>\n";

	ExtentTypeLibrary extentTypeLibrary;
	const ExtentType& extentType =
			 extentTypeLibrary.registerTypeR(extentXMLDescription);

	/*
	 * A sink is a wrapper to a DataSeries output file. Create
	 * a sink and write out the extent type extent.
	 */
	DataSeriesSink outfileSink(argv[5]);
	outfileSink.writeExtentLibrary(extentTypeLibrary);

	/*
	 * Now create an output module that
	 * processes extents using ExtentSeries (iterator)
	 * and put them into the sink.
	 */
	ExtentSeries extentSeries;
	OutputModule outputModule(outfileSink, extentSeries, extentType, 1024);

	/*
	 * These are handles to the fields in the
	 * "current record" of extentSeries
	 */
	Int64Field timestamp(extentSeries, "timestamp");
	ByteField op_code(extentSeries, "op_code");
	Int64Field size(extentSeries, "size");
	Variable32Field filename(extentSeries, "filename");

	/*
	 * Initiate a new record in extentSeries
	 */
	outputModule.newRecord();

	/*
	 * Set new records of the extentSeries
	 */
	timestamp.set((int64_t)atoi(argv[1]) );
	op_code.set((char)atoi(argv[2]));
	size.set((int64_t)atoi(argv[3]));
	filename.set(argv[4]);

	/*
	 * Ask output module to finilize the file.
	 */
	outputModule.flushExtent();
	outputModule.close();

	return 0;
}
