/*
 * Copyright (c) 2011-2012 Santhosh Kumar Koundinya
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2011-2012 Erez Zadok
 * Copyright (c) 2011-2012 Geoff Kuenning
 * Copyright (c) 2011-2012 Stony Brook University
 * Copyright (c) 2011-2012 Harvey Mudd College
 * Copyright (c) 2011-2012 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <sqlite3.h>

#define HANDLE_SQLITE_RC(db, rc)  if (rc != SQLITE_OK) { \
                                         fprintf(stderr, "Error %d: %s.\n", \
                                             rc, sqlite3_errmsg(db)); \
                                         goto cleanup; \
                                     }

#define SQL_CREATE_TABLE  "create table block_trace(rownum integer, " \
				"timestamp real, opcode integer, " \
				"offset integer, iosize integer);"

#define SQL_INSERT "insert into block_trace(rownum, timestamp, " \
		"opcode, offset, iosize) values (%d, %lf, %d, %d, %d);"

#define SQL_CREATE_INDEX_ROWNUM "create unique index if not exists " \
				"block_trace_rownum on block_trace (rownum);"

#define SQL_SELECT_1 "select opcode, offset, iosize from block_trace " \
			"where timestamp < 1000 order by rownum;"

#define SQL_SELECT_2 "select timestamp, opcode, iosize from block_trace " \
		"order by iosize, offset, opcode, timestamp limit 1024;"

#define OPERATION_READ  0
#define OPERATION_WRITE 1
#define SECTOR_SIZE     512

static void show_usage(const char *pgmname)
{
	fprintf(stderr, "Usage: %s trace_filename db_filename\n", pgmname);
	fprintf(stderr, "Trace should be in the SPC format.\n");
	fprintf(stderr, "Example: %s /foo/bar/baz.trc /foo/bar/baz.db\n", pgmname);
}

int create_table(sqlite3 *db)
{
	int rc;

	assert(db);

	rc = sqlite3_exec(db, SQL_CREATE_TABLE, NULL, NULL, NULL);
	HANDLE_SQLITE_RC(db, rc);

cleanup:
	return rc;
}

int insert_data(sqlite3 *db, const char *ASU, const char *LBA, const char *size,
	const char *opcode, const char *timestamp)
{
	int rc;
	static int rownum = 1;
	static char sql_insert[4096];
	double time;
	int op, offset, iosize;

	assert(db);

	time = atof(timestamp);
	op = *opcode == 'r' || *opcode == 'R' ? OPERATION_READ : OPERATION_WRITE;
	offset = atoi(LBA) * SECTOR_SIZE;
	iosize = atoi(size);

	snprintf(sql_insert, sizeof(sql_insert) - 1, SQL_INSERT, rownum, time,
		op, offset, iosize);

	rc = sqlite3_exec(db, sql_insert, NULL, NULL, NULL);
	HANDLE_SQLITE_RC(db, rc);
	rownum++;

cleanup:
	return rc;
}

int load_data(sqlite3 *db, const char *trace_filename)
{
	int rc;
	char *line_buffer = NULL;
	size_t line_buffer_size;
	FILE *trace_file = NULL;
	ssize_t chars_read;

	assert(db);
	assert(trace_filename);

	trace_file = fopen(trace_filename, "r");
	if (!trace_file) {
		fprintf(stderr, "Unable to open trace file '%s': " 
			"%s (errno = %d).\n", trace_filename,
			strerror(errno), errno);
		rc = -1;
		goto cleanup;
	}

	while ((chars_read =
			getline(&line_buffer, &line_buffer_size,
					trace_file)) > 0) {
		char *ASU, *LBA, *size, *opcode, *timestamp;
		char *saveptr;

		/* Remove the newline character */
		line_buffer[chars_read - 1] = '\n';
		ASU = strtok_r(line_buffer, ",", &saveptr);
		LBA = strtok_r(NULL, ",", &saveptr);
		size = strtok_r(NULL, ",", &saveptr);
		opcode = strtok_r(NULL, ",", &saveptr);
		timestamp = strtok_r(NULL, ",", &saveptr);

		rc = insert_data(db, ASU, LBA, size, opcode, timestamp);
		if (rc)
			goto cleanup;
	}

	/* All is well */
	rc = 0;

cleanup:
	if (trace_file)
		fclose(trace_file);

	if (line_buffer)
		free(line_buffer);

	return rc;
}

int create_indexes(sqlite3 *db)
{
	int rc;

	rc = sqlite3_exec(db, SQL_CREATE_INDEX_ROWNUM, NULL, NULL, NULL);
	HANDLE_SQLITE_RC(db, rc);

cleanup:
	return rc;
}

int iterate(sqlite3 *db, const char *sql_select)
{
	int rc;
	sqlite3_stmt *stmt = NULL;

	rc = sqlite3_prepare_v2(db, sql_select, strlen(sql_select), &stmt, NULL);
	HANDLE_SQLITE_RC(db, rc);

	fprintf(stdout, "Query: %s\n", sql_select);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		fprintf(stdout, "%d\t%d\t%d\n", sqlite3_column_int(stmt, 0),
			sqlite3_column_int(stmt, 1), sqlite3_column_int(stmt, 2));
	}

cleanup:
	if (stmt)
		sqlite3_finalize(stmt); /* Don't handle errors here or we'll recurse */

	return rc;
}

int main(int argc, char *argv[])
{
	char *trace_filename = NULL;
	char *db_filename = NULL;
	sqlite3 *db = NULL;
	int rc;

	if (argc < 3) {
		show_usage(argv[0]);
		return EXIT_FAILURE;
	}

	trace_filename = argv[1];
	db_filename = argv[2];

	/* Open the database */
	rc = sqlite3_open_v2(db_filename, &db, SQLITE_OPEN_CREATE |
			SQLITE_OPEN_READWRITE, NULL);
	HANDLE_SQLITE_RC(db, rc);

	/* Create a table to hold trace records */
	rc = create_table(db);
	if (rc)
	   goto cleanup;

	/* Load data */
	rc = load_data(db, trace_filename);
	if (rc)
		goto cleanup;

	/* Create indexes */
	rc = create_indexes(db);
	if (rc)
		goto cleanup;

	/* Examples of iterations */
	iterate(db, SQL_SELECT_1);
	fprintf(stdout, "\n");
	iterate(db, SQL_SELECT_2);

cleanup:
	if (db)
		sqlite3_close(db);

	return rc;
}
