/*
 * FieldSpace.cpp
 *
 * Copyright (c) 2011 Jack Ma
 * Copyright (c) 2011 Vasily Tarasov
 * Copyright (c) 2011 Santhosh Kumar Koundinya
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

#include "FieldSpace.hpp"
#include <stdexcept>

using namespace std;

FieldSpace::FieldSpace()
{
}

void FieldSpace::addFields(const string &syscallName, ExtentSeries &series)
{
	const ExtentType * extenttype = series.getType();
	for	(uint32_t i = 0; i < extenttype->getNFields(); i++) {
		const string &fieldName = extenttype->getFieldName(i);
		bool nullable = extenttype->getNullable(fieldName);

		switch ((ExtentType::fieldType) extenttype->getFieldType(fieldName)) {
			case ExtentType::ft_bool :
				addField(syscallName, fieldName, new BoolField(
						series, fieldName, nullable), ExtentType::ft_bool);
				break;
			case ExtentType::ft_byte :
				addField(syscallName, fieldName, new ByteField(
						series, fieldName, nullable), ExtentType::ft_byte);
				break;
			case ExtentType::ft_int32 :
				addField(syscallName, fieldName, new Int32Field(
						series, fieldName, nullable), ExtentType::ft_int32);
				break;
			case ExtentType::ft_int64 :
				addField(syscallName, fieldName, new Int64Field(
						series, fieldName, nullable), ExtentType::ft_int64);
				break;
			case ExtentType::ft_double :
				addField(syscallName, fieldName, new DoubleField(
						series, fieldName, nullable), ExtentType::ft_double);
				break;
			case ExtentType::ft_variable32 :
				addField(syscallName, fieldName, new Variable32Field(
						series, fieldName, nullable),
						ExtentType::ft_variable32);
				break;
			default:
				cerr << "Unsupported field type: " << fieldName
					<< endl;
				break;
		}
	}
}

void FieldSpace::addField(const string &syscallName, const string &fieldName,
		void* fieldObj, ExtentType::fieldType fieldType)
{
	fieldSpace[syscallName][fieldName] = make_pair(fieldObj, fieldType);
}

template <typename FType>
void FieldSpace::doSetNull(string &syscallName, string &fieldName)
{
	/* Look up if field is nullable */
	((FType*)(fieldSpace[syscallName][fieldName].first))->setNull();
}

template <typename FType, typename EType>
void FieldSpace::doSet(string &syscallName, string &fieldName, void* val)
{
	((FType*)(fieldSpace[syscallName][fieldName].first))->set(*(EType*)val);
}

template <typename FType, typename EType>
EType FieldSpace::doGet(string &syscallName, string &fieldName)
{
	return ((FType*)(fieldSpace[syscallName][fieldName].first))->val();
}

void FieldSpace::setNullField(string &syscallName, string &fieldName)
{
	switch (fieldSpace[syscallName][fieldName].second) {
		case ExtentType::ft_bool:
			doSetNull<BoolField>(syscallName, fieldName);
			break;
		case ExtentType::ft_byte:
			doSetNull<ByteField>(syscallName, fieldName);
			break;
		case ExtentType::ft_int32:
			doSetNull<Int32Field>(syscallName, fieldName);
			break;
		case ExtentType::ft_int64:
			doSetNull<Int64Field>(syscallName, fieldName);
			break;
		case ExtentType::ft_double:
			doSetNull<DoubleField>(syscallName, fieldName);
			break;
		case ExtentType::ft_variable32:
			doSetNull<Variable32Field>(syscallName, fieldName);
			break;
		default:
			stringstream msg;
			msg << "Unsupported field type: " <<
					fieldSpace[syscallName][fieldName].second;
			throw runtime_error(msg.str());
	}
}

void FieldSpace::setField(string &syscallName, string &fieldName, string &val)
{
	void *buffer = malloc(8);
	switch (fieldSpace[syscallName][fieldName].second) {
		case ExtentType::ft_bool:
			if (val == "0")
				*(bool*)buffer = false;
			else
				*(bool*)buffer = true;
			doSet<BoolField, bool>(syscallName, fieldName, buffer);
			break;
		case ExtentType::ft_byte:
			*(ExtentType::byte*)buffer = (char)atoi(val.c_str());
			doSet<ByteField,ExtentType::byte>(syscallName, fieldName, buffer);
			break;
		case ExtentType::ft_int32:
			*(ExtentType::int32*)buffer = atoi(val.c_str());
			doSet<Int32Field,ExtentType::int32>(syscallName, fieldName, buffer);
			break;
		case ExtentType::ft_int64:
			*(ExtentType::int64*)buffer = atoll(val.c_str());
			doSet<Int64Field,ExtentType::int64>(syscallName, fieldName, buffer);
			break;
		case ExtentType::ft_double:
			*(double*)buffer = atof(val.c_str());
			doSet<DoubleField,double>(syscallName, fieldName, buffer);
			break;
		case ExtentType::ft_variable32:
			doSet<Variable32Field,string>(syscallName, fieldName, &val);
			break;
		default:
			stringstream msg;
			msg << "Unsupported field type: " <<
					fieldSpace[syscallName][fieldName].second;
			throw runtime_error(msg.str());
	}
}


double FieldSpace::getField(string &syscallName, string &fieldName)
{
	switch (fieldSpace[syscallName][fieldName].second) {
		case ExtentType::ft_bool:
			if (doGet<BoolField, bool>(syscallName, fieldName))
				return 1;
			else
				return 0;
		case ExtentType::ft_byte:
			return doGet<ByteField,ExtentType::byte>(syscallName, fieldName);
		case ExtentType::ft_int32:
			return doGet<Int32Field,ExtentType::int32>(syscallName, fieldName);
		case ExtentType::ft_int64:
			return doGet<Int64Field,ExtentType::int64>(syscallName, fieldName);
		case ExtentType::ft_double:
			return doGet<DoubleField,double>(syscallName, fieldName);
		case ExtentType::ft_variable32:
			cerr << "Variable32 getField is not supported" << endl;
			return 0;
		default:
			stringstream msg;
			msg << "Unsupported field type: " <<
					fieldSpace[syscallName][fieldName].second;
			throw runtime_error(msg.str());
	}
}

FieldSpace::~FieldSpace()
{
	for(FieldSpaceDataType::iterator fsdti = fieldSpace.begin();
			fsdti != fieldSpace.end(); fsdti++) {
		for(FieldSpaceEntryType::iterator fseti = fsdti->second.begin();
						     fseti != fsdti->second.end();
						     fseti++) {
			switch (fseti->second.second) {
				case ExtentType::ft_bool:
					delete (BoolField*) fseti->second.first;
					break;
				case ExtentType::ft_byte:
					delete (ByteField*) fseti->second.first;
					break;
				case ExtentType::ft_int32:
					delete (Int32Field*) fseti->second.first;
					break;
				case ExtentType::ft_int64:
					delete (Int64Field*) fseti->second.first;
					break;
				case ExtentType::ft_double:
					delete (DoubleField*) fseti->second.first;
					break;
				case ExtentType::ft_variable32:
					delete (Variable32Field*) fseti->second.first;
					break;
				default:
					/* Don't throw an exception in the destructor.
					 * Just log an error. */
					cerr << "Unsupported field type: " << fseti->second.second <<
							"\n";
					break;
			}
		}
	}
}
