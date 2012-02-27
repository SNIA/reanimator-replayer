/*
	csv2ds-extra
	This program is similar to DataSeries's csv2ds utility, but handles
	extents with different types and nullable fields.

	Usage: ./csv2ds-extra <outputfile> <tablefile> <spec_string_file> <inputfiles...>

	<outputfile> name of the dataseries output file
	<tablefile> name of the table file to refer to
	<spec_string_file> name of the file that contains the string that specifies the format of the input file
	<inputfiles...> input csv files

	Limitation: Variable32 fields that may not contain commas.
*/

#include <cassert>

#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include <DataSeries/ExtentType.hpp>
#include <DataSeries/DataSeriesFile.hpp>
#include <DataSeries/DataSeriesModule.hpp>

#include "FieldSpace.hpp"

using namespace std;
/* map<fieldname, pair<nullable, ExtentType> */
typedef map<string, pair<bool, ExtentType::fieldType> > config_table_entry_type;
/* map<extentname, config_table_entry_type> */
typedef map<string,config_table_entry_type > config_table_type;

typedef map<string, OutputModule*> OutputModule_space_type;
typedef map<string, ExtentSeries*> ExtentSeries_space_type;
typedef map<string, vector<string> > specs_type;


config_table_type makeTable(std::ifstream &input) {

	config_table_type table;
	std::string line;
	
	/* Special case for Common fields */
	config_table_entry_type commonfieldmap;
	while (getline(input, line)) {
		vector<string> split_data;
		boost::split(split_data, line, boost::is_any_of("\t"));

		if (split_data.size() != 4) {
			std::cout << "Illegal table file" << std::endl;
			exit(1);
		}
		string extentname = split_data[0];
		string fieldname = split_data[1];
		string nullablestr = split_data[2];
		string fieldtype = split_data[3];

		bool nullable = false;
		if (nullablestr == "1")
			nullable = true;

		ExtentType::fieldType ftype = ExtentType::ft_unknown;
		if (fieldtype == "bool")
			ftype = ExtentType::ft_bool;
		if (fieldtype == "byte")
			ftype = ExtentType::ft_byte;
		if (fieldtype == "int32")
			ftype = ExtentType::ft_int32;
		if (fieldtype == "int64" or fieldtype=="time")
			ftype = ExtentType::ft_int64;
		if (fieldtype == "double")
			ftype = ExtentType::ft_double;
		if (fieldtype == "variable32")
			ftype = ExtentType::ft_variable32;

		if (extentname == "Common")
			commonfieldmap[fieldname] = make_pair(nullable, ftype);
		else if (table.find(extentname) != table.end())
			table[extentname] [fieldname] = make_pair(nullable, ftype);
		else { /* New extent detected */
			table[extentname] = commonfieldmap;
			table[extentname] [fieldname] = make_pair(nullable, ftype);	 	
		}

	}
	return table;
}

specs_type parseSpecs(ifstream &input, config_table_type &table) {
	string specstring, line;
	while (getline(input, line))
		specstring += line;
	specs_type specs;
	vector<string> split_data, specdata, fields, commonfields;
	boost::split(split_data, specstring, boost::is_any_of(";"),
					     boost::token_compress_on);

	if (split_data.back() == "")
		split_data.pop_back();
	
	/* Iterate through each extent group */
	for (vector<string>::iterator split_data_i = split_data.begin();
				 split_data_i != split_data.end();
				 split_data_i++) {

		/* Break up the extentname and the fields */
		boost::split(specdata, *split_data_i, boost::is_any_of("\()"));
			
		if (specdata.size() < 2) {
			std::cerr << "Illegal specs string" << std::endl;
			exit(1);
		}
		
		std::string extentname = specdata[0];
		boost::split(fields, specdata[1], boost::is_any_of(","),
					boost::token_compress_on);
		if (fields.back() == "")
			fields.pop_back();

		if (extentname == "Common")
			commonfields = fields;
		else {
			vector<string> allfields(commonfields);
			allfields.insert(allfields.end(), fields.begin(), fields.end());
			
			/* Check if the name of the extent exist */
			if (table.find(extentname) == table.end()) {
				cerr << "Cannot find extent " << extentname << endl;
				exit(1);
			}
		
			/* Check if all the fields exist*/
			for (vector<string>::iterator iter = allfields.begin();
					    iter != allfields.end();
					    iter++) {
				if (table[extentname].find(*iter) == table[extentname].end()) {
					cerr << "Cannot find field " << extentname << ":" << *iter << endl;
					exit(1);
				}
			}
			specs[extentname] = allfields;
		}
	}
	return specs;
}



class DataSeriesWriteModule {
public:
	/* Disable copy constructor */
	DataSeriesWriteModule(const DataSeriesWriteModule&);

	/* Constructor to set up all extents and fields */
	DataSeriesWriteModule(config_table_type &table, specs_type &specs, const char *outfile)
		: outfileSink(outfile), table_(table), specs_(specs)
	{
		/* Registering extent types to the library */
		ExtentTypeLibrary extentTypeLibrary;
		
		/* Set each extent's size to be 4096 bytes */
		uint32_t target_extent_size = 4096;

		/* loop through each extent and create its fields from xmls */
		for (config_table_type::iterator extent = table_.begin();
						 extent != table_.end();
						 extent++) {
			string extentname = extent->first;

			/* Loading extent XML descriptions from outside file */
			std::ifstream myExtentXMLFile(("xml/" + extentname + ".xml").c_str());
			if (!myExtentXMLFile.is_open()) {
				std::cout << extentname << ": Coud not open xml file!\n";
				exit(1);
			}
			string myExtentXMLDescription, str;
			while (getline(myExtentXMLFile, str))
				myExtentXMLDescription += str + "\n";
			
			/* Register the ExtentXMLDescription */
			const ExtentType &myExtentType =
				 extentTypeLibrary.registerTypeR(myExtentXMLDescription);

			/* Create ExtentSeries, OutPutModule, and fields */
			ExtentSeries *myExtentSeries = new ExtentSeries();
			ExtentSeriesSpace[extentname] = myExtentSeries;
			OutputModuleSpace[extentname] = new OutputModule(outfileSink, *myExtentSeries, myExtentType, target_extent_size);
			fieldSpace.addFields(extentname, *myExtentSeries);
		}

		/* Write out the extent type extent. */
		outfileSink.writeExtentLibrary(extentTypeLibrary);
	}
	
	/* Register the record and field values in string format into fields */
	void updateRecord(string &record, bool quietmode)
	{
		vector<string> split_data;
		boost::split(split_data, record, boost::is_any_of(","));
		if (split_data.back() == "")
			split_data.pop_back();

		if (split_data.size() < 2 or specs_[split_data[0]].size() != split_data.size() - 1) {
			cerr << record << ": Invalid record! Number of fields do not match specification string." << endl;
			exit(1);
		}

		/* First place is the extentname, other fields are values */
		string extentname = split_data[0];
		OutputModuleSpace[extentname]->newRecord();

		/* Create a map from fieldnames to fieldvalue. 
			Iterate through every possible fields (via table_). 
			If the field is in the map, then set value of the
			field. Otherwise set it to null */
			
		map<string,string> fieldvaluemap;
		for (size_t i = 0; i != specs_[extentname].size(); i++) {
			string fieldname = specs_[extentname][i];
			string fieldvalue = split_data[i + 1];
			fieldvaluemap[fieldname] = fieldvalue;
		}

		for (config_table_entry_type::iterator iter = table_[extentname].begin();
						 iter != table_[extentname].end();
						 iter++)
		{
			string fieldname = iter->first;
			bool nullable = iter->second.first;
			if (fieldvaluemap.find(fieldname) != fieldvaluemap.end()) {
				string fieldvalue = fieldvaluemap[fieldname];
				if (fieldvalue != "") {
					fieldSpace.setField(extentname, fieldname, fieldvalue);
					continue;
				}
			}
			if (nullable)
				fieldSpace.setNullField(extentname, fieldname);
			else if (!quietmode) {
				cerr << extentname << ":" << fieldname << " ";
				cerr << "WARNING: Attempting to setNull to a non-nullable field. ";
				cerr << "This field will take on default value instead." << endl;
			}				
		}
	}
	
	
	~DataSeriesWriteModule()
	{
		for (OutputModule_space_type::iterator iter = OutputModuleSpace.begin();
							iter != OutputModuleSpace.end();
							iter++) {
			iter->second->flushExtent();
			iter->second->close();
			delete iter->second;
		}
	
	}

	OutputModule_space_type OutputModuleSpace;
	ExtentSeries_space_type ExtentSeriesSpace;
	FieldSpace fieldSpace;
	/* Sink is a wrapper for a DataSeries output file. */
	DataSeriesSink outfileSink;
private:
	config_table_type table_;
	specs_type specs_;
};


int main(int argc, char *argv[]) {
	bool quietmode = false;
	if (argc < 4) {
		std::cout << "Wrong usage!\n";
		std::cout << "Usage: " << argv[0] << " [-q] <outputfile> <tablefile> <spec_string_file> <inputfiles...> \n";
		return 1;
	}

	int current_arg = 1;

	if (!strcmp("-q", argv[current_arg])) {
		quietmode = true;
		current_arg++;
	}

	const char *outfile = argv[current_arg++];
	std::ifstream tablefile(argv[current_arg++]);
	std::ifstream specfile(argv[current_arg++]);

	if (!tablefile.is_open()) {
		std::cout << "Could not open table file!\n";
		return 1;
	}

	if (!specfile.is_open()) {
		std::cout << "Could not open spec string file!\n";
		return 1;
	}

	/* Load the configuration table */
	config_table_type table = makeTable(tablefile);

	/* Parse specification string */
	specs_type specs = parseSpecs(specfile, table);

	/* Create Tables and Fields */
	DataSeriesWriteModule writemod = DataSeriesWriteModule(table, specs, outfile);
	for (int i = current_arg; i < argc; i++) {
		std::ifstream inputfile(argv[i]);
		cout << "Processing input file " << argv[i] << endl;
		if (!inputfile.is_open()) {
			cout << "Could not open input trace file!" << endl;
			continue;
		}
		string line;
		while (getline(inputfile, line))
			writemod.updateRecord(line, quietmode);
	}

}
