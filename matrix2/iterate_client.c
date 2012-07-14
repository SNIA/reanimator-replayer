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
 * Print a histogram of read iosize distribution in the last 10 seconds.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include "feature_matrix.h"
#include "feature_matrix_iterator.h"

#define OPERATION_READ ((int8_t) 0)
#define OPERATION_WRITE ((int8_t) 1)

static void show_usage(const char *pgmname)
{
	fprintf(stdout, "Usage: %s <database file> <matrix name>\n",
			pgmname);
	fprintf(stdout, "Example: %s ./some.db fin1\n", pgmname);
}

static int check_matrix(t2mfm *fmh)
{
	int rc;
	t2mfm_vec dim_meta_vec;
	char *dim_names[] = {"opcode", "offset", "iosize"};
	int i;

	assert(fmh);

	rc = t2mfm_get_dim_meta(fmh, &dim_meta_vec);
	T2MFM_HANDLE_RC(fmh, rc, cleanup);

	/* Check the number of dimensions */
	if (dim_meta_vec.n_elements != 3) {
		fprintf(stderr, "Incorrect number of dimenisons. Expected 3, found %zu"
				".\n", dim_meta_vec.n_elements);
		rc = 1;
		goto cleanup;
	}

	/* Check the names of each dimension */
	for (i = 0; i < sizeof(dim_names) / sizeof(char *); i++) {
		if (!strcmp(dim_meta_vec.pp_data[i], dim_names[i])) {
			fprintf(stderr, "Dimension mismatch at %d: Expected '%s' found "
					"'%s'.\n", i, dim_names[i],
					((t2mfm_dim_meta *)dim_meta_vec.pp_data[i])->name);
			rc = 1;
			goto cleanup;
		}
	}

	/* All is well */
	rc = 0;
cleanup:
	return rc;
}

static int select_op(t2mfm_itr *fmitrh, int8_t op)
{
	int rc;
	t2mfm_dim_data start, end;

	assert(fmitrh);

	start.INT1 = op;
	end.INT1 = op;

	rc = t2mfm_add_filter(fmitrh, 0, start, end);
	T2MFM_HANDLE_ITR_RC(fmitrh, rc, cleanup);

cleanup:
	return rc;
}

static int add_projections_iosize(t2mfm_itr *fmitrh)
{
	int rc;
	int dims_to_project[] = {0, 1};
	int i;

	assert(fmitrh);

	/* Project dimensions */
	for (i = 0; i < sizeof(dims_to_project) / sizeof(int); i++) {
		rc = t2mfm_add_projection(fmitrh, dims_to_project[i]);
		T2MFM_HANDLE_ITR_RC(fmitrh, rc, cleanup);
	}

cleanup:
	return rc;
}

static int add_projections_offset(t2mfm_itr *fmitrh)
{
	int rc;
	int dims_to_project[] = {0, 2};
	int i;

	assert(fmitrh);

	/* Project dimensions */
	for (i = 0; i < sizeof(dims_to_project) / sizeof(int); i++) {
		rc = t2mfm_add_projection(fmitrh, dims_to_project[i]);
		T2MFM_HANDLE_ITR_RC(fmitrh, rc, cleanup);
	}

cleanup:
	return rc;
}


static int add_projections_op(t2mfm_itr *fmitrh)
{
	int rc;
	int dims_to_project[] = {1, 2};
	int i;

	assert(fmitrh);

	/* Project dimensions */
	for (i = 0; i < sizeof(dims_to_project) / sizeof(int); i++) {
		rc = t2mfm_add_projection(fmitrh, dims_to_project[i]);
		T2MFM_HANDLE_ITR_RC(fmitrh, rc, cleanup);
	}

cleanup:
	return rc;
}


static int add_ordering_iosize(t2mfm_itr *fmitrh)
{
	int rc;

	assert(fmitrh);

	rc = t2mfm_set_itr_order(fmitrh, 2, T2MFM_ASC);
	T2MFM_HANDLE_ITR_RC(fmitrh, rc, out);

out:
	return rc;
}

static int add_ordering_offset(t2mfm_itr *fmitrh)
{
	int rc;

	assert(fmitrh);

	rc = t2mfm_set_itr_order(fmitrh, 1, T2MFM_ASC);
	T2MFM_HANDLE_ITR_RC(fmitrh, rc, out);

out:
	return rc;
}

static int print_histogram(t2mfm_itr *fmitrh, FILE *p_file)
{
	int rc;
	t2mfm_vec dim_data_vec;

	assert(fmitrh);

	rc = t2mfm_itr_begin(fmitrh, &dim_data_vec);
	T2MFM_HANDLE_ITR_RC(fmitrh, rc, cleanup);

	assert(dim_data_vec.n_elements == 2);

	while ((rc = t2mfm_itr_next(fmitrh)) == T2MFM_NEW_ROW) {
		fprintf(p_file, "%" PRIi64 ", %" PRIi64 "\n",
			T2MFM_GET_DIM_DATA(dim_data_vec, 0, INT8),
				T2MFM_GET_DIM_DATA(dim_data_vec, 1, INT8));
	}

	if (rc != T2MFM_NO_MORE_ROWS)
		T2MFM_HANDLE_ITR_RC(fmitrh, rc, cleanup);

	rc = 0;

cleanup:
	fflush(p_file);
	return rc;
}

static int read_write_ratio(t2mfm *fmh)
{
	t2mfm_itr *fmitrh = NULL;
	int rc;

	/* Allocate a new iterator */
	rc = t2mfm_itr_alloc(fmh, &fmitrh);
	T2MFM_HANDLE_RC(fmh, rc, free_itr);

	/* Project all dimensions except operations */
	rc = add_projections_op(fmitrh);
	if (rc)
		goto free_itr;

	/* Iterate over operations and print the result */
	fprintf(stdout, "Read/write ratio:\n");
	rc = print_histogram(fmitrh, stdout);

free_itr:
	t2mfm_itr_free(fmitrh);

	return rc;
}

static int iosize_distribution(t2mfm *fmh)
{
	t2mfm_itr *fmitrh = NULL;
	int rc;

	/* Allocate a new iterator */
	rc = t2mfm_itr_alloc(fmh, &fmitrh);
	T2MFM_HANDLE_RC(fmh, rc, free_itr);

	/* Project all dimensions except iosize */
	rc = add_projections_iosize(fmitrh);
	if (rc)
		goto free_itr;

	/* Order by iosize */
	rc = add_ordering_iosize(fmitrh);
	if (rc)
		goto free_itr;

	/* Iterate over all iosizes and print the result */
	fprintf(stdout, "I/O size distribution:\n");
	rc = print_histogram(fmitrh, stdout);

free_itr:
	t2mfm_itr_free(fmitrh);

	return rc;
}

static int read_iosize_distribution(t2mfm *fmh)
{
	t2mfm_itr *fmitrh = NULL;
	int rc;

	/* Allocate a new iterator */
	rc = t2mfm_itr_alloc(fmh, &fmitrh);
	T2MFM_HANDLE_RC(fmh, rc, free_itr);

	rc = select_op(fmitrh, OPERATION_READ);
	if (rc)
		goto free_itr;

	/* Project all dimensions except iosize */
	rc = add_projections_iosize(fmitrh);
	if (rc)
		goto free_itr;

	/* Order by iosize */
	rc = add_ordering_iosize(fmitrh);
	if (rc)
		goto free_itr;

	/* Iterate over all iosizes and print the result */
	fprintf(stdout, "READ I/O size distribution:\n");
	rc = print_histogram(fmitrh, stdout);

free_itr:
	t2mfm_itr_free(fmitrh);

	return rc;
}

static int write_iosize_distribution(t2mfm *fmh)
{
	t2mfm_itr *fmitrh = NULL;
	int rc;

	/* Allocate a new iterator */
	rc = t2mfm_itr_alloc(fmh, &fmitrh);
	T2MFM_HANDLE_RC(fmh, rc, free_itr);

	rc = select_op(fmitrh, OPERATION_WRITE);
	if (rc)
		goto free_itr;

	/* Project all dimensions except iosize */
	rc = add_projections_iosize(fmitrh);
	if (rc)
		goto free_itr;

	/* Order by iosize */
	rc = add_ordering_iosize(fmitrh);
	if (rc)
		goto free_itr;

	/* Iterate over all iosizes and print the result */
	fprintf(stdout, "WRITE I/O size distribution:\n");
	rc = print_histogram(fmitrh, stdout);

free_itr:
	t2mfm_itr_free(fmitrh);

	return rc;
}

static int read_offset_distribution(t2mfm *fmh)
{
	t2mfm_itr *fmitrh = NULL;
	int rc;

	/* Allocate a new iterator */
	rc = t2mfm_itr_alloc(fmh, &fmitrh);
	T2MFM_HANDLE_RC(fmh, rc, free_itr);

	rc = select_op(fmitrh, OPERATION_READ);
	if (rc)
		goto free_itr;

	/* Project all dimensions except offset */
	rc = add_projections_offset(fmitrh);
	if (rc)
		goto free_itr;

	/* Order by offset */
	rc = add_ordering_offset(fmitrh);
	if (rc)
		goto free_itr;

	/* Iterate over all iosizes and print the result */
	fprintf(stdout, "READ offset distribution:\n");
	rc = print_histogram(fmitrh, stdout);

free_itr:
	t2mfm_itr_free(fmitrh);

	return rc;
}

static int write_offset_distribution(t2mfm *fmh)
{
	t2mfm_itr *fmitrh = NULL;
	int rc;

	/* Allocate a new iterator */
	rc = t2mfm_itr_alloc(fmh, &fmitrh);
	T2MFM_HANDLE_RC(fmh, rc, free_itr);

	rc = select_op(fmitrh, OPERATION_WRITE);
	if (rc)
		goto free_itr;

	/* Project all dimensions except offset */
	rc = add_projections_offset(fmitrh);
	if (rc)
		goto free_itr;

	/* Order by offset */
	rc = add_ordering_offset(fmitrh);
	if (rc)
		goto free_itr;

	/* Iterate over all offsets and print the result */
	fprintf(stdout, "WRITE offset distribution:\n");
	rc = print_histogram(fmitrh, stdout);

free_itr:
	t2mfm_itr_free(fmitrh);

	return rc;
}



int main(int argc, char *argv[])
{
	int rc;
	char *backing_store_file;
	char *matrix_name;
	t2mfm *fmh = NULL;

	if (argc < 3) {
		show_usage(argv[0]);
		return EXIT_FAILURE;
	}

	backing_store_file = argv[1];
	matrix_name = argv[2];

	/* Open the matrix for iterations */
	rc = t2mfm_open(backing_store_file,
			matrix_name, T2MFM_OPEN_ITERATE, &fmh);
	T2MFM_HANDLE_RC(fmh, rc, close_matrix);

	/* Check to see if we have a handle to what we really want */
	rc = check_matrix(fmh);
	if (rc)
		goto close_matrix;

	/* read/write ratio */
	rc = read_write_ratio(fmh);
	fprintf(stdout, "\n");

	/* iosize distributio */
	rc = iosize_distribution(fmh);
	fprintf(stdout, "\n");

	/* read iosize distribution */
	rc = read_iosize_distribution(fmh);
	fprintf(stdout, "\n");

	/* write iosize distribution */
	rc = write_iosize_distribution(fmh);
	fprintf(stdout, "\n");

	/* read offset distribution */
	rc = read_offset_distribution(fmh);
	fprintf(stdout, "\n");

	/* write offset distribution */
	rc = write_offset_distribution(fmh);

close_matrix:
	t2mfm_close(fmh);

	return rc;
}
