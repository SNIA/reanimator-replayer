
#include <iostream>
#include <stdint.h>
#include <map>
#include <unistd.h>
#include <utility>
#include <DataSeries/ExtentType.hpp>
#include <DataSeries/DataSeriesFile.hpp>

#ifndef FIELDSPACE_HPP
#define FIELDSPACE_HPP

/* map<fieldname, pair<address, ExtentType> */
typedef std::map<std::string, std::pair<void*, ExtentType::fieldType> > field_space_entry_type;

/* map<syscallname, field_space_entry_type> */
typedef std::map<std::string,field_space_entry_type> field_space_data_type;


using namespace std;

class FieldSpace {
public:

	FieldSpace();

	// Disable copy constructor.
	FieldSpace(FieldSpace &fspace);
	
	~FieldSpace();

	void addFields(string &syscallname, ExtentSeries &series);
	
	void addField(std::string &syscallname, std::string &fieldname, void* field_obj, ExtentType::fieldType field_type);
	
	template <typename FType>
	void doSetNull(std::string &syscallname, std::string &fieldname);

	template <typename FType, typename EType>
	void doSet(std::string &syscallname, std::string &fieldname, void* val);

	template <typename FType, typename EType>
	EType doGet(std::string &syscallname, std::string &fieldname);

	void setNullField(std::string &syscallname, std::string &fieldname);

	void setField(std::string &syscallname, std::string &fieldname, std::string &val);

	double getField(std::string &syscallname, std::string &fieldname);

private:
	field_space_data_type fieldspace;
};


#endif
