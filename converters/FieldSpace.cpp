

#include "FieldSpace.hpp"

using namespace std;

FieldSpace::FieldSpace()
{
}

void FieldSpace::addFields(string &syscallname, ExtentSeries &series)
{
	string fieldName;
	const ExtentType * extenttype = series.getType();
	for	(uint32_t i = 0; i < extenttype->getNFields(); i++)
	{
		fieldName = extenttype->getFieldName(i);
		bool nullable = extenttype->getNullable(fieldName);

		switch ((ExtentType::fieldType) extenttype->getFieldType(fieldName))
		{
			case ExtentType::ft_bool :
				addField(syscallname, fieldName, new BoolField(series, fieldName, nullable), ExtentType::ft_bool);
				break;
			case ExtentType::ft_byte :
				addField(syscallname, fieldName, new ByteField(series, fieldName, nullable), ExtentType::ft_byte);
				break;
			case ExtentType::ft_int32 :
				addField(syscallname, fieldName, new Int32Field(series, fieldName, nullable), ExtentType::ft_int32);
				break;
			case ExtentType::ft_int64 :
				addField(syscallname, fieldName, new Int64Field(series, fieldName, nullable), ExtentType::ft_int64);
				break;
			case ExtentType::ft_double :
				addField(syscallname, fieldName, new DoubleField(series, fieldName, nullable), ExtentType::ft_double);
				break;
			case ExtentType::ft_variable32 :
				addField(syscallname, fieldName, new Variable32Field(series, fieldName, nullable), ExtentType::ft_variable32);
				break;
			default:
				cerr << "Ignoring unsupported field type: " << fieldName << endl;
		}
	}
}


void FieldSpace::addField(string &syscallname, string &fieldname, void* field_obj, ExtentType::fieldType field_type)
{
	fieldspace[syscallname][fieldname] = make_pair(field_obj, field_type);
}

template <typename FType>
void FieldSpace::doSetNull(string &syscallname, string &fieldname)
{
	/* Look up if field is nullable */
	((FType*)(fieldspace[syscallname][fieldname].first))->setNull();
}

template <typename FType, typename EType>
void FieldSpace::doSet(string &syscallname, string &fieldname, void* val)
{
	((FType*)(fieldspace[syscallname][fieldname].first))->set(*(EType*)val);
}

template <typename FType, typename EType>
EType FieldSpace::doGet(string &syscallname, string &fieldname)
{
	return ((FType*)(fieldspace[syscallname][fieldname].first))->val();
}

void FieldSpace::setNullField(string &syscallname, string &fieldname)
{
	switch (fieldspace[syscallname][fieldname].second) {
		case ExtentType::ft_bool:
			doSetNull<BoolField>(syscallname, fieldname);
			break;
		case ExtentType::ft_byte:
			doSetNull<ByteField>(syscallname, fieldname);
			break;
		case ExtentType::ft_int32:
			doSetNull<Int32Field>(syscallname, fieldname);
			break;
		case ExtentType::ft_int64:
			doSetNull<Int64Field>(syscallname, fieldname);
			break;
		case ExtentType::ft_double:
			doSetNull<DoubleField>(syscallname, fieldname);
			break;
		case ExtentType::ft_variable32:
			doSetNull<Variable32Field>(syscallname, fieldname);
			break;
		default:
			cerr << "Unknown FieldType detected!" << endl;
			exit(1);
	}
}

void FieldSpace::setField(string &syscallname, string &fieldname, string &val)
{
	void *buffer = malloc(8);
	switch (fieldspace[syscallname][fieldname].second) {
		case ExtentType::ft_bool:
			if (val == "0")
				*(bool*)buffer = true;
			else
				*(bool*)buffer = false;
			doSet<BoolField, bool>(syscallname, fieldname, buffer);
			break;
		case ExtentType::ft_byte:
			*(ExtentType::byte*)buffer = val.c_str()[0];
			doSet<ByteField,ExtentType::byte>(syscallname, fieldname, buffer);
			break;
		case ExtentType::ft_int32:
			*(ExtentType::int32*)buffer = atoi(val.c_str());
			doSet<Int32Field,ExtentType::int32>(syscallname, fieldname, buffer);
			break;
		case ExtentType::ft_int64:
			*(ExtentType::int64*)buffer = atof(val.c_str());
			doSet<Int64Field,ExtentType::int64>(syscallname, fieldname, buffer);
			break;
		case ExtentType::ft_double:
			*(double*)buffer = atof(val.c_str());
			doSet<DoubleField,double>(syscallname, fieldname, buffer);
			break;
		case ExtentType::ft_variable32:
			doSet<Variable32Field,string>(syscallname, fieldname, &val);
			break;
		default:
			cerr << "Unknown FieldType detected!" << endl;
			exit(1);
	}
}


double FieldSpace::getField(string &syscallname, string &fieldname)
{
	switch (fieldspace[syscallname][fieldname].second) {
		case ExtentType::ft_bool:
			if (doGet<BoolField, bool>(syscallname, fieldname))
				return 1;
			else
				return 0;
		case ExtentType::ft_byte:
			return doGet<ByteField,ExtentType::byte>(syscallname, fieldname);
		case ExtentType::ft_int32:
			return doGet<Int32Field,ExtentType::int32>(syscallname, fieldname);
		case ExtentType::ft_int64:
			return doGet<Int64Field,ExtentType::int64>(syscallname, fieldname);
		case ExtentType::ft_double:
			return doGet<DoubleField,double>(syscallname, fieldname);
		case ExtentType::ft_variable32:
			cerr << "Variable32 getField is not supported" << endl;
			return 0;
		default:
			cerr << "Unknown FieldType detected!" << endl;
			exit(1);
	}
}

FieldSpace::~FieldSpace()
{

	for(field_space_data_type::iterator iter = fieldspace.begin();
					    iter != fieldspace.end();
					    iter++)
	{
		for(field_space_entry_type::iterator iter2 = iter->second.begin();
						     iter2 != iter->second.end();
						     iter2++)
		{
			switch (iter2->second.second) {
				case ExtentType::ft_bool:
					delete (BoolField*) iter2->second.first;
					break;
				case ExtentType::ft_byte:
					delete (ByteField*) iter2->second.first;
					break;
				case ExtentType::ft_int32:
					delete (Int32Field*) iter2->second.first;
					break;
				case ExtentType::ft_int64:
					delete (Int64Field*) iter2->second.first;
					break;
				case ExtentType::ft_double:
					delete (DoubleField*) iter2->second.first;
					break;
				case ExtentType::ft_variable32:
					delete (Variable32Field*) iter2->second.first;
					break;
				default:
					cerr << "Unknown FieldType detected!" << endl;
					exit(1);
			}
		}
	}
}


