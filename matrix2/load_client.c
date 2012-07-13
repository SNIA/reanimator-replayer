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
 * A file that can load SPC Traces (Like Financial1 and Financial2) into
 * the matrix.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>

#include "feature_matrix.h"

#define OPERATION_READ 0
#define OPERATION_WRITE 1
#define SECTOR_SIZE 512

static void show_usage(const char *pgmname)
{
	fprintf(stdout, "Usage: %s <SPC trace filename> "
			"<database file> <matrix name>\n", pgmname);
	fprintf(stdout, "Example: %s trace-examples/Financial1-snippet.spc "
			"./some.db fin1\n", pgmname);
}

static int insert_data(t2mfm *fmh, const t2mfm_vec *p_dim_data_vec, const char *ASU,
		const char *LBA, const char *size, const char *opcode,
		const char *timestamp)
{
	int rc;
	int op;

	assert(fmh);
	assert(p_dim_data_vec && (p_dim_data_vec->n_elements == 3));

	op = *opcode == 'r' || *opcode == 'R' ? OPERATION_READ : OPERATION_WRITE;

	T2MFM_SET_DIM_DATA(*p_dim_data_vec, 0, INT1, (int8_t)op);
	#define OFFSET_GRANULARITY (1024 * 1024 * 1024)
	T2MFM_SET_DIM_DATA(*p_dim_data_vec, 1, INT8, atoi(LBA) * SECTOR_SIZE / OFFSET_GRANULARITY);
	#undef OFFSET_GRANULARITY
	T2MFM_SET_DIM_DATA(*p_dim_data_vec, 2, INT8, atol(size));

	rc = t2mfm_insert(fmh);
	T2MFM_HANDLE_RC(fmh, rc, cleanup);

cleanup:
	return rc;
}

static int load_data(t2mfm *fmh, const char *trace_filename)
{
	int rc;
	char *line_buffer = NULL;
	size_t line_buffer_size;
	FILE *trace_file = NULL;
	ssize_t chars_read;
	t2mfm_vec dim_data_vec;

	assert(fmh);
	assert(trace_filename);

	trace_file = fopen(trace_filename, "r");
	if (!trace_file) {
		rc = errno;
		fprintf(stderr, "Unable to open trace file '%s': "
			"%s (errno = %d).\n", trace_filename,
					strerror(errno), errno);
		goto tracefile_close;
	}

	rc = t2mfm_insert_begin(fmh, &dim_data_vec);
	T2MFM_HANDLE_RC(fmh, rc, insert_end);

	/* We expect dimensions opcode, offset and iosize */
	assert(dim_data_vec.n_elements == 3);

	while ((chars_read = getline(&line_buffer, &line_buffer_size, trace_file))
			> 0) {
		char *ASU, *LBA, *size, *opcode, *timestamp;
		char *saveptr;

		/* Remove the newline character */
		line_buffer[chars_read - 1] = '\n';
		ASU = strtok_r(line_buffer, ",", &saveptr);
		LBA = strtok_r(NULL, ",", &saveptr);
		size = strtok_r(NULL, ",", &saveptr);
		opcode = strtok_r(NULL, ",", &saveptr);
		timestamp = strtok_r(NULL, ",", &saveptr);

		rc = insert_data(fmh, &dim_data_vec, ASU, LBA, size, opcode, timestamp);
		if (rc)
			goto insert_end;
	}

	/* All is well */
	rc = 0;

insert_end:
	t2mfm_insert_end(fmh); /* XXX: Don't handle errors here or we'll recurse */

tracefile_close:
	if (trace_file)
		fclose(trace_file);

	if (line_buffer)
		free(line_buffer);

	return rc;
}

static int add_dimensions(t2mfm *fmh)
{
	int rc;
	/* Declare new dimensions, statically initializing their meta-data */
	t2mfm_dim_meta opcode = {0, "opcode",
			"the opcode: 0 for reads, 1 for writes", T2MFM_INT1,
			T2MFM_FALSE};
	t2mfm_dim_meta offset = {1, "offset", "offset in bytes", T2MFM_INT8,
			T2MFM_FALSE};
	t2mfm_dim_meta iosize = {2, "iosize", "iosize in bytes", T2MFM_INT8,
			T2MFM_FALSE};

	assert(fmh);

	/* Begin adding dimensions */
	rc = t2mfm_add_dim_meta_begin(fmh);
	T2MFM_HANDLE_RC(fmh, rc, cleanup);

	/* Add individual dimensions */
	rc = t2mfm_add_dim_meta(fmh, &opcode);
	T2MFM_HANDLE_RC(fmh, rc, cleanup);

	rc = t2mfm_add_dim_meta(fmh, &offset);
	T2MFM_HANDLE_RC(fmh, rc, cleanup);

	rc = t2mfm_add_dim_meta(fmh, &iosize);
	T2MFM_HANDLE_RC(fmh, rc, cleanup);

cleanup:
	t2mfm_add_dim_meta_end(fmh); /* XXX: No error handling */

	return rc;
}

int main(int argc, char *argv[])
{
	int rc;
	char *trace_filename;
	char *backing_store_file;
	char *matrix_name;
	t2mfm *fmh = NULL;

	if (argc < 4) {
		show_usage(argv[0]);
		return EXIT_FAILURE;
	}

	/* Initialize user supplied arguments */
	trace_filename = argv[1];
	backing_store_file = argv[2];
	matrix_name = argv[3];

	/* Create a new feature matrix */
	rc = t2mfm_open(backing_store_file, matrix_name,
					T2MFM_OPEN_CREATE, &fmh);
	if (rc)
		goto cleanup;

	/* Add dimensions to newly created matrix */
	rc = add_dimensions(fmh);
	if (rc)
		goto cleanup;

	/* Load SPC trace */
	rc = load_data(fmh, trace_filename);
	if (rc)
		goto cleanup;

cleanup:
	t2mfm_close(fmh);
	return rc;
}
