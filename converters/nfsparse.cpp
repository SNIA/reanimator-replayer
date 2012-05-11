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
 * We assume that Ethernet packets are captured in PCAP format using tcpdump
 * tool.  Tshark  network protocol has a capability to decode NFS packets
 * from Ethernet frames.  Below command can be used:
 *
 * # tshark -r <pcap_file> -R nfs -t e -z proto,colinfo,rpc.xid,rpc.xid [-z proto,colinfo,nfs.full_name,nfs.full_name] > <output_file>
 *
 * [-z proto,colinfo,nfs.full_name,nfs.full_name] is an optinal filter
 * that instructs tshark to deduce full file name for approriate NFS
 * procedures. It is supported only starting from version 1.6.5.
 *

 * nfsparse is usually invoked by nfstrace2d.sh to convert trace in a PCAP
 * format to the DataSeries format.  nfsparse produces a new CSV file that is
 * then fed to a csv2ds-extra tool.
 *
 * Usage: nfsparse <pcap_file> <output_csv_file>
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

bool processMkdirRequest(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	vector<string> diropargs;
	const char *extentName = "mkdir_request";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 13) {
		boost::split(diropargs, fields[9], boost::is_any_of("/"));
		string &dh = diropargs[0];
		string dir_handle = dh.substr(3, dh.length());
		string &file_name = diropargs[1];

		string &timestamp = fields[0];
		string &rpc_xid = fields[12];
		string path_name = (fields.size() >= 16) ? fields[15] : ",";

		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << dir_handle << "," << file_name 
			<< "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processMkdirReply(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "mkdir_reply";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);
 
	if (fields.size() >= 15) {
 		string &timestamp = fields[0];
		string &rpc_xid = fields[14];
		string dir = (fields.size() >= 18) ? fields[17] : "";
		string par_dir = (fields.size() >= 21) ? fields[20] : ",";
 
 		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
 			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << dir << "," << par_dir;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processAccessRequest(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "access_request";
	uint32_t field_offset = 0;
	int read, lookup, modify, extend, del, execute;
	read = lookup = modify = extend = del = execute = 0;

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 12) {
		string &timestamp = fields[0];
		string &fh = fields[9];

		for(int i=11; i<=16; i++ ){
			if (string::npos != fields[i].find("RD"))
				read = 1;
			else if (string::npos != fields[i].find("LU"))
				lookup = 1;
			else if (string::npos != fields[i].find("MD"))
				modify = 1;
			else if (string::npos != fields[i].find("XT"))
				extend = 1;
			else if (string::npos != fields[i].find("DL"))
				del = 1;
			else if (string::npos != fields[i].find("XE"))
				execute = 1;

			if (string::npos != fields[i].find("]")) {
				field_offset = i;
				break;
			}
		}

		string &rpc_xid = fields[field_offset+3];
		string path_name = (fields.size() >= (field_offset + 7)) ? fields[field_offset+6] : ",";
		
		string file_handle = fh.substr(3, fh.length()-4);

		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << file_handle << "," << read << 
			lookup << modify << extend << del << execute << "," 
			<< path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processAccessReply(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "access_reply";
	int alw_read, alw_lookup, alw_modify, alw_extend, alw_del, alw_execute;
	int dny_read, dny_lookup, dny_modify, dny_extend, dny_del, dny_execute;
	alw_read = alw_lookup = alw_modify = alw_extend = alw_del = alw_execute = 0;
	dny_read = dny_lookup = dny_modify = dny_extend = dny_del = dny_execute = 0;
	uint32_t field_offset = 0;

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 14) {
		string &timestamp = fields[0];
		
		if (string::npos != inRow.find("Allowed") && string::npos != inRow.find("Denied")){
			for(int i=14; i<=19; i++){
				if (string::npos != fields[i].find("RD"))
					dny_read = 1;
				else if (string::npos != fields[i].find("LU"))
					dny_lookup = 1;
				else if (string::npos != fields[i].find("MD"))
					dny_modify = 1;
				else if (string::npos != fields[i].find("XT"))
					dny_extend = 1;
				else if (string::npos != fields[i].find("DL"))
					dny_del = 1;
				else if (string::npos != fields[i].find("XE"))
					dny_execute = 1;

				if (string::npos != fields[i].find("]")) {
					field_offset = i;
					break;
				}
			}

			for(uint32_t i=field_offset+2; i<=field_offset+7; i++){
				if (string::npos != fields[i].find("RD"))
					alw_read = 1;
				else if (string::npos != fields[i].find("LU"))
					alw_lookup = 1;
				else if (string::npos != fields[i].find("MD"))
					alw_modify = 1;
				else if (string::npos != fields[i].find("XT"))
					alw_extend = 1;
				else if (string::npos != fields[i].find("DL"))
					alw_del = 1;
				else if (string::npos != fields[i].find("XE"))
					alw_execute = 1;

				if (string::npos != fields[i].find("]")) {
					field_offset = i;
					break;
				}
			}

		}else if(string::npos != inRow.find("Allowed")){
			for(int i=13; i<=18; i++){
				if (string::npos != fields[i].find("RD"))
					alw_read = 1;
				else if (string::npos != fields[i].find("LU"))
					alw_lookup = 1;
				else if (string::npos != fields[i].find("MD"))
					alw_modify = 1;
				else if (string::npos != fields[i].find("XT"))
					alw_extend = 1;
				else if (string::npos != fields[i].find("DL"))
					alw_del = 1;
				else if (string::npos != fields[i].find("XE"))
					alw_execute = 1;

				if (string::npos != fields[i].find("]")) {
					field_offset = i;
					break;
				}
			}

		}else if(string::npos != inRow.find("Denied")){
			for(int i=14; i<=19; i++){
				if (string::npos != fields[i].find("RD"))
					dny_read = 1;
				else if (string::npos != fields[i].find("LU"))
					dny_lookup = 1;
				else if (string::npos != fields[i].find("MD"))
					dny_modify = 1;
				else if (string::npos != fields[i].find("XT"))
					dny_extend = 1;
				else if (string::npos != fields[i].find("DL"))
					dny_del = 1;
				else if (string::npos != fields[i].find("XE"))
					dny_execute = 1;

				if (string::npos != fields[i].find("]")) {
					field_offset = i;
					break;
				}
			}
		}
		
		string &rpc_xid = fields[field_offset+3];
		string path_name = (fields.size() >= (field_offset + 7)) ? fields[field_offset+6] : ",";

		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << alw_read << alw_lookup << 
			alw_modify << alw_extend << alw_del << alw_execute 
			<< "," << dny_read << dny_lookup << dny_modify << 
			dny_extend << dny_del << dny_execute << "," << 
			path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processGetAttrRequest(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "getattr_request";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 13) {
		string &timestamp = fields[0];
		string &fh = fields[9];
		string &rpc_xid = fields[12];
		string path_name = (fields.size() >= 16) ? fields[15] : ",";
		
		string file_handle = fh.substr(3, fh.length());

		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << file_handle << ","  << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processGetAttrReply(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "getattr_reply";
	uint32_t field_offset = 14;
	int file_type = 0;

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (string::npos != inRow.find(" Regular File ")) {
		field_offset = 14;
		file_type = 1;
	}
	else if (string::npos != inRow.find(" Directory ")) {
		field_offset = 13;
		file_type = 2;
	}

	if (fields.size() >= field_offset + 6) {
		string &timestamp = fields[0];
		string &raw_mode = fields[field_offset];
		string raw_uid = fields[field_offset+1];
		string raw_gid = fields[field_offset+2];
		string &rpc_xid = fields[field_offset+5];
		
		string path_name = (fields.size() >= (field_offset + 9)) ? fields[field_offset+8] : ",";

		string mode = raw_mode.substr(5, raw_mode.length());
		string uid = raw_uid.substr(4, raw_uid.length());
		string gid = raw_gid.substr(4, raw_gid.length());
		
		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << file_type << "," << mode << "," 
			<< uid << "," << gid << "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}	
	return ret;
}

bool processLookupRequest(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	vector<string> diropargs;
	const char *extentName = "lookup_request";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 13) {
		boost::split(diropargs, fields[9], boost::is_any_of("/"));
		string &dh = diropargs[0];
		string dir_handle = dh.substr(3, dh.length());
		string &file_name = diropargs[1];

		string &timestamp = fields[0];
		string &rpc_xid = fields[12];
		string path_name = (fields.size() >= 16) ? fields[15] : ",";

		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << dir_handle << "," << file_name 
			<< "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}	
	return ret;
}

bool processLookupReply(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "lookup_reply";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 16) {
		string &timestamp = fields[0];
		string &rpc_xid = fields[15];

		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		
		if (string::npos != inRow.find("Error:")) {
			string &error_field = fields[12];
			string path_name = (fields.size() >= 19) ? fields[18] : ",";
			
			string error = error_field.substr(6, error_field.length());

			formattedRow << extentName << "," << tfracs_timestamp << ","
				<< rpc_xid << "," << error << "," << path_name;

		} else if (string::npos != inRow.find("FH:")) {
			string &fh = fields[12];
			string path_name1 = (fields.size() >= 19) ? fields[18] : ",";
			//string &path_name2 = (fields.size() >= 25) ? fields[24] : "";
			
			string file_handle = fh.substr(3, fh.length());

			/*formattedRow << extentName << "," << tfracs_timestamp << ","
				<< rpc_xid << "," << file_handle << "," << 
				path_name1 << "*" << path_name2;*/
			
			formattedRow << extentName << "," << tfracs_timestamp << ","
				<< rpc_xid << "," << file_handle << "," << 
				path_name1;

		}
		
		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processCreateRequest(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	vector<string> diropargs;
	const char *extentName = "create_request";
	int mode = 0;

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 14) {
		if (string::npos != inRow.find(" Mode:UNCHECKED ")) {
			mode = 0;
			boost::split(diropargs, fields[9], boost::is_any_of("/"));
			string &dh = diropargs[0];
			string dir_handle = dh.substr(3, dh.length());
			string &file_name = diropargs[1];

			string &timestamp = fields[0];
			string &rpc_xid = fields[13];
			string path_name = (fields.size() >= 17) ? fields[16] : ",";

			uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
				(((uint64_t)1)<<32));

			stringstream formattedRow;
			formattedRow << extentName << "," << tfracs_timestamp << ","
				<< rpc_xid << "," << dir_handle << "," << file_name 
				<< "," << mode << "," << path_name;

			outRow = formattedRow.str();
		}else if (string::npos != inRow.find(" Mode:GUARDED ")) {
			mode = 1;
		}else if (string::npos != inRow.find(" Mode:EXCLUSIVE ")) {
			mode = 2;
		}

	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processCreateReply(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "create_reply";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);
 
	if (fields.size() >= 15) {
		string &timestamp = fields[0];

		string &rpc_xid = fields[14];
		string dir = (fields.size() >= 18) ? fields[17] : "";
		string par_dir = (fields.size() >= 21) ? fields[20] : ",";

		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << dir << "," << par_dir;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}	
	return ret;
}

bool processRemoveRequest(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	vector<string> diropargs;
	const char *extentName = "remove_request";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 13) {
		boost::split(diropargs, fields[9], boost::is_any_of("/"));
		string &dh = diropargs[0];
		string dir_handle = dh.substr(3, dh.length());
		string &file_name = diropargs[1];

		string &timestamp = fields[0];
		string &rpc_xid = fields[12];
		string path_name = (fields.size() >= 16) ? fields[15] : ",";

		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << dir_handle << "," << file_name 
			<< "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processRemoveReply(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "remove_reply";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 15) {
		string &timestamp = fields[0];
		string &rpc_xid = fields[14];
		string path_name = (fields.size() >= 18) ? fields[17] : ",";

		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processCommitRequest(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "commit_request";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 13) {
		string &timestamp = fields[0];
		string &fh = fields[9];
		string &rpc_xid = fields[12];
		string path_name = (fields.size() >= 16) ? fields[15] : ",";

		string file_handle = fh.substr(3, fh.length());
		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << file_handle << "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processCommitReply(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "commit_reply";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 15) {
		string &timestamp = fields[0];
		string &rpc_xid = fields[14];
		string path_name = (fields.size() >= 18) ? fields[17] : ",";

		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}	
	return ret;
}

bool processPathconfRequest(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "pathconf_request";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 13) {
		string &timestamp = fields[0];
		string &fh = fields[9];
		string &rpc_xid = fields[12];
		string path_name = (fields.size() >= 16) ? fields[15] : ",";

		string file_handle = fh.substr(3, fh.length());
		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << file_handle << ","  << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processPathconfReply(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "pathconf_reply";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 15) {
		string &timestamp = fields[0];
		string &rpc_xid = fields[14];
		string path_name = (fields.size() >= 18) ? fields[17] : ",";
		
		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processFsinfoRequest(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "fsinfo_request";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);
	
	if (fields.size() >= 13) {
		string &timestamp = fields[0];
		string &fh = fields[9];
		string &rpc_xid = fields[12];
		string path_name = (fields.size() >= 16) ? fields[15] : ",";
		
		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));
		string file_handle = fh.substr(3, fh.length());

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << file_handle << "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processFsinfoReply(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "fsinfo_reply";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 15) {
		string &timestamp = fields[0];
		string &rpc_xid = fields[14];
		string path_name = (fields.size() >= 18) ? fields[17] : ",";
		
		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processReadRequest(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "read_request";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 15) {
		string &timestamp = fields[0];
		string &fh = fields[9];
		string &rpc_xid = fields[14];
		string path_name = (fields.size() >= 18) ? fields[17] : ",";

		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));
		string file_handle = fh.substr(3, fh.length());
		uint64_t offset = (uint64_t)atoll(fields[10].substr(7, fields[10].length()).c_str());
		uint64_t length = (uint64_t)atoll(fields[11].substr(4, fields[11].length()).c_str());

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << file_handle << "," << offset << "," 
			<< length << "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processReadReply(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "read_reply";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 16) {
		string &timestamp = fields[0];
		string &rpc_xid = fields[15];
		string path_name = (fields.size() >= 19) ? fields[18] : ",";
		
		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));
		uint64_t length = (uint64_t)atoll(fields[12].substr(4, fields[12].length()).c_str());

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << length << "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processWriteRequest(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "write_request";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	if (fields.size() >= 16) {
		string &timestamp = fields[0];
		string &fh = fields[9];
		int unstable = (fields[12] ==  "UNSTABLE") ? 1 : 0;
		int data_sync = (fields[12] == "DATA_SYNC") ? 1 : 0;
		int file_sync = (fields[12] == "FILE_SYNC") ? 1 : 0;
		string &rpc_xid = fields[15];
		string path_name = (fields.size() >= 19) ? fields[18] : ",";

		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));
		string file_handle = fh.substr(3, fh.length());
		uint64_t offset = (uint64_t)atoll(fields[10].substr(7, fields[10].length()).c_str());
		uint64_t length = (uint64_t)atoll(fields[11].substr(4, fields[11].length()).c_str());

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," <<file_handle << "," << offset << "," 
			<< length << "," << unstable << "," << data_sync << "," 
			<< file_sync << "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processWriteReply(const string &inRow, string &outRow)
{
	bool ret = true;
	vector<string> fields;
	const char *extentName = "write_reply";

	stringstream tokenizer(inRow);
	string num;
	while (tokenizer >> num)
		fields.push_back(num);

	
	if (fields.size() >= 17) {
		string &timestamp = fields[0];
		int unstable = (fields[13] ==  "UNSTABLE") ? 1 : 0;
		int data_sync = (fields[13] == "DATA_SYNC") ? 1 : 0;
		int file_sync = (fields[13] == "FILE_SYNC") ? 1 : 0;
		string &rpc_xid = fields[16];
		string path_name = (fields.size() >= 20) ? fields[19] : ",";

		uint64_t tfracs_timestamp = (uint64_t)(atof(timestamp.c_str()) *
			(((uint64_t)1)<<32));
		uint64_t length = (uint64_t)atoll(fields[12].substr(4, fields[12].length()).c_str());

		stringstream formattedRow;
		formattedRow << extentName << "," << tfracs_timestamp << ","
			<< rpc_xid << "," << length << "," << unstable << ","
			<< data_sync << "," << file_sync << "," << path_name;

		outRow = formattedRow.str();
	} else {
		clog << "NFS: Malformed record: '" << inRow <<"\n";
		return false;
	}
	return ret;
}

bool processRow(const string &inRow, string &outRow, bool &write_outRow)
{
	boost::regex CALLEXPR(".*NFS[\\s]+[0-9]+[\\s]+V3[\\s]+[A-Z]+[\\s]+Call.*");
	boost::regex REPLYEXPR(".*NFS[\\s]+[0-9]+[\\s]+V3[\\s]+[A-Z]+[\\s]+Reply.*");
	bool ret = true;
	bool (*processRequest)(const string &inRow, string &outRow) = NULL;
	bool (*processReply) (const string &inRow, string &outRow) = NULL;
	
	outRow = "";
	

	if (boost::regex_match(inRow, CALLEXPR)) {
		if (string::npos != inRow.find(" WRITE ")) {
			processRequest = processWriteRequest;
			write_outRow = true;
		} else if (string::npos != inRow.find(" READ ")) {
			processRequest = processReadRequest;
			write_outRow = true;
		} else if (string::npos != inRow.find(" FSINFO ")) {
			processRequest = processFsinfoRequest;
			write_outRow = true;
		} else if (string::npos != inRow.find(" PATHCONF ")) {
			processRequest = processPathconfRequest;
			write_outRow = true;
		} else if (string::npos != inRow.find(" COMMIT ")) {
			processRequest = processCommitRequest;
			write_outRow = true;
		}else if (string::npos != inRow.find(" MKDIR ")) {
			processRequest = processMkdirRequest;
			write_outRow = true;
		}else if (string::npos != inRow.find(" LOOKUP ")) {
			processRequest = processLookupRequest;
			write_outRow = true;
		}else if (string::npos != inRow.find(" GETATTR ")) {
			processRequest = processGetAttrRequest;
			write_outRow = true;
		}else if (string::npos != inRow.find(" CREATE ")) {
			processRequest = processCreateRequest;
			write_outRow = true;
		}else if (string::npos != inRow.find(" REMOVE ")) {
			processRequest = processRemoveRequest;
			write_outRow = true;
		}else if (string::npos != inRow.find(" ACCESS ")) {
			processRequest = processAccessRequest;
			write_outRow = true;
		}

		if (NULL != processRequest) { 
			if(!processRequest(inRow, outRow)) {
				clog << "NFS: Malformed record: '" << inRow <<"\n";
				return false;
			}
		}
	} else if (boost::regex_match(inRow, REPLYEXPR)) {
		if (string::npos != inRow.find(" WRITE ")) {
			processReply = processWriteReply;
			write_outRow = true;
		} else if (string::npos != inRow.find(" READ ")) {
			processReply = processReadReply;
			write_outRow = true;
		} else if (string::npos != inRow.find(" FSINFO ")) {
			processReply = processFsinfoReply;
			write_outRow = true;
		} else if (string::npos != inRow.find(" PATHCONF ")) {
			processReply = processPathconfReply;
			write_outRow = true;
		} else if (string::npos != inRow.find(" COMMIT ")) {
			processReply = processCommitReply;
			write_outRow = true;
		}else if (string::npos != inRow.find(" MKDIR ")) {
			processReply = processMkdirReply;
			write_outRow = true;
		}else if (string::npos != inRow.find(" LOOKUP ")) {
			processReply = processLookupReply;
			write_outRow = true;
		}else if (string::npos != inRow.find(" GETATTR ")) {
			processReply = processGetAttrReply;
			write_outRow = true;
		}else if (string::npos != inRow.find(" CREATE ")) {
			processReply = processCreateReply;
			write_outRow = true;
		}else if (string::npos != inRow.find(" REMOVE ")) {
			processReply = processRemoveReply;
			write_outRow = true;
		}else if (string::npos != inRow.find(" ACCESS ")) {
			processReply = processAccessReply;
			write_outRow = true;
		}

		if (NULL != processReply) { 
			if(!processReply(inRow, outRow)) {
				clog << "NFS: Malformed record: '" << inRow <<"\n";
				return false;
			}
		}

	}
	return ret;
}

void showUsage(const char *pgmname)
{
	cerr << "Usage:   " << pgmname << " <input file> <output file>\n";
	cerr << "Example: " << pgmname << " /foo/bar /foo/baz\n";
}

int main(int argc, char *argv[])
{
	const char *inFileName;
	const char *outFileName;
	ifstream inFile;
	ofstream outFile;
	string inRow;
	string outRow;
	int rowNum;
	int ret = EXIT_SUCCESS;
	bool write_outRow = false;
	
	if (argc < 3) {
		showUsage(argv[0]);
		return EXIT_FAILURE;
	}

	inFileName = argv[1];
	outFileName = argv[2];

	inFile.open(inFileName, ios_base::in);
	if (!inFile.is_open()) {
		cerr << "Unable to open input file '" << inFileName << "'.\n";
		ret = EXIT_FAILURE;
		goto cleanup;
	}

	outFile.open(outFileName, ios_base::out);
	if (!outFile.is_open()) {
		cerr << "Unable to open output file '" << outFileName << "'.\n";
		ret = EXIT_FAILURE;
		goto cleanup;
	}

	rowNum = 0;
	while(getline(inFile, inRow)) {
		rowNum++;
		if (!processRow(inRow, outRow, write_outRow)) {
			cerr << "Error processing record " << rowNum << ".\n";
			ret = EXIT_FAILURE;
			continue;
		}
		if (write_outRow) {
			outFile << outRow << "\n";
			write_outRow = false;
		}
	}

cleanup:
	if (inFile.is_open()) {
		inFile.close();
	}

	if (outFile.is_open()) {
		outFile.close();
	}

	/* Delete the out file if we are not successful */
	if (ret != EXIT_SUCCESS) {
		remove(outFileName); /* Don't handle errors here */
	}

	return ret;
}
