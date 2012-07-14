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

void show_usage(const char *pgmname)
{
	fprintf(stdout, "Usage: %s <database file> <matrix name>\n",
			pgmname);
	fprintf(stdout, "Example: %s ./some.db fin1\n", pgmname);
}

int check_matrix(t2mfm *fmh)
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

int set_timerange(t2mfm_itr *fmitrh, int range)
{
	int rc;
	t2mfm_dim_data start_time, end_time;

	assert(fmitrh);
	assert(range > 0);

	/* Find the max time */
	rc = t2mfm_get_stat(fmitrh->fmh, 0, T2MFM_STAT_MAX, &end_time);
	T2MFM_HANDLE_ITR_RC(fmitrh, rc, cleanup);

	start_time.REAL = end_time.REAL - range; /* Go back @range seconds */

	rc = t2mfm_add_filter(fmitrh, 0, start_time, end_time);
	T2MFM_HANDLE_ITR_RC(fmitrh, rc, cleanup);

cleanup:
	return rc;
}

int select_op(t2mfm_itr *fmitrh, int8_t op)
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

int add_projections(t2mfm_itr *fmitrh)
{
	int rc;
	int dims_to_project[] = {0, 1};
	int i;

	assert(fmitrh);

	/* Project dimensions */
	for (i = 0; i < sizeof(dims_to_project) / sizeof(int); i++) {
		rc = t2mfm_add_projection(fmitrh, i);
		T2MFM_HANDLE_ITR_RC(fmitrh, rc, cleanup);
	}

cleanup:
	return rc;
}

int add_ordering(t2mfm_itr *fmitrh)
{
	int rc;

	assert(fmitrh);

	rc = t2mfm_set_itr_order(fmitrh, 2, T2MFM_ASC);
	T2MFM_HANDLE_ITR_RC(fmitrh, rc, out);

out:
	return rc;
}

int print_histogram(t2mfm_itr *fmitrh, FILE *p_file)
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

int main(int argc, char *argv[])
{
	int rc;
	char *backing_store_file;
	char *matrix_name;
	t2mfm *fmh = NULL;
	t2mfm_itr *fmitrh = NULL;

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

	/* Allocate a new iterator */
	rc = t2mfm_itr_alloc(fmh, &fmitrh);
	T2MFM_HANDLE_RC(fmh, rc, free_itr);

	rc = select_op(fmitrh, OPERATION_READ);
	if (rc)
		goto free_itr;

	/* Project all dimensions except iosize */
	rc = add_projections(fmitrh);
	if (rc)
		goto free_itr;

	/* Order by iosize */
	rc = add_ordering(fmitrh);
	if (rc)
		goto free_itr;

	/* Iterate over all iosizes and print the result */
	rc = print_histogram(fmitrh, stdout);
	if (rc)
		goto free_itr;

	/* All is well */
	rc = EXIT_SUCCESS;

free_itr:
	t2mfm_itr_free(fmitrh);

close_matrix:
	t2mfm_close(fmh);

	return rc;
}
