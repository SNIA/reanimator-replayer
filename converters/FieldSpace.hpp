/*
 * FieldSpace.hpp
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

/* TODO: Add a description of the class here. */

#include <iostream>
#include <stdint.h>
#include <map>
#include <unistd.h>
#include <utility>
#include <DataSeries/ExtentType.hpp>
#include <DataSeries/DataSeriesFile.hpp>

#ifndef FIELD_SPACE_HPP_
#define FIELD_SPACE_HPP_

/* map<fieldName, pair<address, ExtentType> */
typedef std::map<std::string, std::pair<void*, ExtentType::fieldType> >
	FieldSpaceEntryType;

/* map<syscallName, field_space_entry_type> */
typedef std::map<std::string, FieldSpaceEntryType> FieldSpaceDataType;

using namespace std;

class FieldSpace
{
public:
	FieldSpace();

	void addFields(const string &syscallName, ExtentSeries &series);

	void addField(const std::string &syscallName, const std::string &fieldName,
			void* fieldObj, ExtentType::fieldType fieldType);

	template <typename FType>
	void doSetNull(std::string &syscallName, std::string &fieldName);

	template <typename FType, typename EType>
	void doSet(std::string &syscallName, std::string &fieldName, void* val);

	template <typename FType, typename EType>
	EType doGet(std::string &syscallName, std::string &fieldName);

	void setNullField(std::string &syscallName, std::string &fieldName);

	void setField(std::string &syscallName, std::string &fieldName,
			std::string &val);

	double getField(std::string &syscallName, std::string &fieldName);

	~FieldSpace();

private:
	/* Disable copy and assignment. */
	FieldSpace(FieldSpace &fspace);
	FieldSpace &operator=(const FieldSpace &fspace);

	FieldSpaceDataType fieldSpace;
};

#endif /* FIELD_SPACE_HPP_ */
