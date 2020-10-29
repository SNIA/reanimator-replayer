/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include "feature_matrix.h"
#include "feature_matrix_iterator.h"
#include "feature_matrix_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef struct filter_info
{
	t2mfm_dim_meta *dim_meta;
	t2mfm_dim_data start;
	t2mfm_dim_data end;
} filter_info;

typedef struct ordering_info
{
	t2mfm_dim_meta *dim_meta;
	t2mfm_itr_order order;
} ordering_info;

static inline void set_itr_errmsg(t2mfm_itr *fmitrh, const char *errmsg)
{
	if (!fmitrh)
		return;

	if (errmsg) {
		if (fmitrh->errmsg)
			free(fmitrh->errmsg);
		fmitrh->errmsg = strdup(errmsg);
	}

	if (fmitrh->fmh->db) {
		if (fmitrh->sqlite_errmsg)
			free(fmitrh->sqlite_errmsg);
		fmitrh->sqlite_errmsg = strdup(sqlite3_errmsg(fmitrh->fmh->db));
	}
}

t2mfm_itr *alloc_fmitrh(t2mfm *fmh)
{
	t2mfm_itr *fmitrh;
	int i;

	if (!fmh)
		return NULL;

	fmitrh = (t2mfm_itr *) calloc(1, sizeof(t2mfm_itr));
	if (!fmitrh) {
		set_errmsg(fmh, "out of memory");
		fmh->rc = T2MFM_ENOMEM;
		return NULL;
	}

	fmitrh->fmh = fmh;

	fmitrh->projection_info_vec.n_elements = fmh->dim_meta_vec.n_elements;
	/* TODO: Handle out of memory */
	fmitrh->projection_info_vec.pp_data = (void **) calloc(
			fmitrh->projection_info_vec.n_elements, sizeof(void *));
	for (i = 0; i < fmitrh->projection_info_vec.n_elements; i++)
		fmitrh->projection_info_vec.pp_data[i] = (void *) T2MFM_FALSE;

	fmh->itr_head = list_add_tail(fmh->itr_head, fmitrh);

	return fmitrh;
}

void free_fmitrh(t2mfm_itr *fmitrh)
{
	if (!fmitrh)
		return;

	fmitrh->fmh->itr_head = list_free_node(fmitrh->fmh->itr_head, fmitrh,
			T2MFM_TRUE);
}

int t2mfm_add_filter(t2mfm_itr *fmitrh, int dim_ordinal, t2mfm_dim_data start,
		t2mfm_dim_data end)
{
	filter_info *fi;

	if (!fmitrh)
		return T2MFM_EINVAL;

	if (dim_ordinal < 0 || dim_ordinal >= fmitrh->fmh->dim_meta_vec.n_elements)
	{
		set_itr_errmsg(fmitrh, "invalid dim_ordinal");
		return (fmitrh->rc = T2MFM_EINVAL);
	}

	/* TODO: Handle out of memory */
	fi = (filter_info *) calloc(1, sizeof(filter_info));
	fi->dim_meta = (t2mfm_dim_meta *)
			fmitrh->fmh->dim_meta_vec.pp_data[dim_ordinal];
	fi->start = start;
	fi->end = end;

	fmitrh->filter_info_head = list_add_tail(fmitrh->filter_info_head,
			fi);

	return (fmitrh->rc = T2MFM_OK);
}

int t2mfm_add_projection(t2mfm_itr *fmitrh, int dim_ordinal)
{
	if (!fmitrh)
		return T2MFM_EINVAL;

	if (dim_ordinal < 0 || dim_ordinal >= fmitrh->fmh->dim_meta_vec.n_elements)
	{
		set_itr_errmsg(fmitrh, "invalid dim_ordinal");
		return (fmitrh->rc = T2MFM_EINVAL);
	}

	fmitrh->projection_info_vec.pp_data[dim_ordinal] = (void *) T2MFM_TRUE;

	return (fmitrh->rc = T2MFM_OK);
}

int t2mfm_set_itr_order(t2mfm_itr *fmitrh, int dim_ordinal,
		t2mfm_itr_order order)
{
	ordering_info *oi;

	if (!fmitrh)
		return T2MFM_EINVAL;

	if (dim_ordinal < 0 || dim_ordinal >= fmitrh->fmh->dim_meta_vec.n_elements)
	{
		set_itr_errmsg(fmitrh, "invalid dim_ordinal");
		return (fmitrh->rc = T2MFM_EINVAL);
	}

	/* TODO: Handle out of memory */
	oi = (ordering_info *) calloc(1, sizeof(oi));
	oi->dim_meta = (t2mfm_dim_meta *)
			fmitrh->fmh->dim_meta_vec.pp_data[dim_ordinal];
	oi->order = order;
	fmitrh->ordering_info_head = list_add_tail(fmitrh->ordering_info_head,
			oi);

	return (fmitrh->rc = T2MFM_OK);
}

int t2mfm_itr_begin(t2mfm_itr *fmitrh, t2mfm_vec *p_dim_data_vec)
{
	const size_t sqlbuf_size = 4096;
	char sqlbuf[sqlbuf_size];
	int chars_written = 0;
	int n_dim_projected = 0;
	int i,j;

	if (!fmitrh)
		return T2MFM_EINVAL;

	if (!p_dim_data_vec) {
		set_itr_errmsg(fmitrh, "NULL p_dim_data_vec");
		return (fmitrh->rc = T2MFM_EINVAL);
	}

	/* select */
	chars_written = snprintf(sqlbuf, sqlbuf_size, "select ");

	for (i = 0; i < fmitrh->projection_info_vec.n_elements; i++)
		if (((t2mfm_bool)fmitrh->projection_info_vec.pp_data[i]) == T2MFM_TRUE)
			n_dim_projected++;

	/* columns */
	if (n_dim_projected == 0) {
		get_dim_names(fmitrh->fmh, sqlbuf + chars_written, sqlbuf_size -
				(size_t) chars_written, &chars_written);
		if (chars_written < sqlbuf_size)
			chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size -
					(size_t) chars_written, ", t2mfm_frequency");
	} else if (n_dim_projected < fmitrh->fmh->dim_meta_vec.n_elements){
		t2mfm_bool is_first_dim = T2MFM_TRUE;
		for (i = 0; i < fmitrh->projection_info_vec.n_elements &&
			chars_written < sqlbuf_size; i++) {
			if (((t2mfm_bool)fmitrh->projection_info_vec.pp_data[i]) ==
					T2MFM_FALSE) {
				chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size -
						(size_t) chars_written, is_first_dim == T2MFM_TRUE ?
						"%s" : ", %s",((t2mfm_dim_meta *)
						fmitrh->fmh->dim_meta_vec.pp_data[i])->name);
				if (is_first_dim == T2MFM_TRUE)
					is_first_dim = T2MFM_FALSE;
			}
		}

		if (chars_written < sqlbuf_size)
			chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size -
					(size_t) chars_written, ", sum(t2mfm_frequency)");
	} else {
		set_itr_errmsg(fmitrh, "all dimensions projected");
		return (fmitrh->rc = T2MFM_EINVAL_PROJECTION);
	}

	/* from table */
	if (chars_written < sqlbuf_size)
		chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size - (size_t)
				chars_written, " from %s", fmitrh->fmh->matrix_name);

	/* where */
	if (fmitrh->filter_info_head) {
		t2mfm_dll_node *cur = fmitrh->filter_info_head;
		while (cur && chars_written < sqlbuf_size)  {
			chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size -
					(size_t) chars_written, cur == fmitrh->filter_info_head ?
					" where (%s >= ? and %s <= ?)" :
					" and (%s >= ? and %s <= ?)", ((filter_info *) cur->data)
					->dim_meta->name, ((filter_info *) cur->data)->dim_meta
					->name);
			cur = cur->next;
		}
	}

	/* group by */
	if (n_dim_projected > 0) {
		t2mfm_bool is_first_dim = T2MFM_TRUE;
		for (i = 0; i < fmitrh->projection_info_vec.n_elements &&
			chars_written < sqlbuf_size; i++) {
			if (((t2mfm_bool)fmitrh->projection_info_vec.pp_data[i]) ==
					T2MFM_FALSE) {
				chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size -
						(size_t) chars_written, is_first_dim == T2MFM_TRUE ?
						" group by %s" : ", %s",((t2mfm_dim_meta *)
						fmitrh->fmh->dim_meta_vec.pp_data[i])->name);
				if (is_first_dim == T2MFM_TRUE)
					is_first_dim = T2MFM_FALSE;
			}
		}
	}

	/* order by */
	if (fmitrh->ordering_info_head) {
		t2mfm_dll_node *cur = fmitrh->ordering_info_head;
		while (cur && chars_written < sqlbuf_size)  {
			chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size -
					(size_t) chars_written, cur == fmitrh->ordering_info_head ?
					" order by %s %s" : ", %s %s", ((ordering_info *) cur->data)
					->dim_meta->name, ((ordering_info *) cur->data)->order ==
					T2MFM_ASC ? "asc" : "desc");
			cur = cur->next;
		}
	} else {
		if (chars_written < sqlbuf_size)
			chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size -
					(size_t) chars_written, " order by _rowid_ asc");
	}

	/* terminal ';' */
	if (chars_written < sqlbuf_size - 1)
		sqlbuf[chars_written++] = ';';

	fmitrh->sqlite_rc = sqlite3_prepare_v2(fmitrh->fmh->db, sqlbuf, -1,
			&fmitrh->select_stmt, NULL);
	if (fmitrh->sqlite_rc != SQLITE_OK) {
		set_itr_errmsg(fmitrh, "internal error: itr begin: sqlite error");
		fmitrh->rc = T2MFM_SQLITE_ERROR;
		goto cleanup;
	}

	/* bind parameters for the where clause */
	if (fmitrh->filter_info_head) {
		t2mfm_dll_node *cur = fmitrh->filter_info_head;
		int bind_index = 1;
		while (cur && chars_written < sqlbuf_size)  {
			filter_info *fi = (filter_info *) cur->data;
			switch (fi->dim_meta->datatype) {
			case T2MFM_INT1:
				fmitrh->sqlite_rc = sqlite3_bind_int(fmitrh->select_stmt,
						bind_index++, fi->start.INT1);
				break;
			case T2MFM_INT4:
				fmitrh->sqlite_rc = sqlite3_bind_int(fmitrh->select_stmt,
						bind_index++, fi->start.INT4);
				break;
			case T2MFM_INT8:
				fmitrh->sqlite_rc = sqlite3_bind_int64(fmitrh->select_stmt,
						bind_index++, fi->start.INT8);
				break;
			case T2MFM_REAL:
				fmitrh->sqlite_rc = sqlite3_bind_double(fmitrh->select_stmt,
						bind_index++, fi->start.REAL);
				break;
			default:
				assert(0 && "illegal datatype");
				break;
			}

			if (fmitrh->sqlite_rc != SQLITE_OK) {
				set_itr_errmsg(fmitrh, "itr_begin: bind: sqlite internal "
						"error");
				fmitrh->rc = T2MFM_SQLITE_ERROR;
				goto cleanup;
			}

			switch (fi->dim_meta->datatype) {
			case T2MFM_INT1:
				fmitrh->sqlite_rc = sqlite3_bind_int(fmitrh->select_stmt,
						bind_index++, fi->end.INT1);
				break;
			case T2MFM_INT4:
				fmitrh->sqlite_rc = sqlite3_bind_int(fmitrh->select_stmt,
						bind_index++, fi->end.INT4);
				break;
			case T2MFM_INT8:
				fmitrh->sqlite_rc = sqlite3_bind_int64(fmitrh->select_stmt,
						bind_index++, fi->end.INT8);
				break;
			case T2MFM_REAL:
				fmitrh->sqlite_rc = sqlite3_bind_double(fmitrh->select_stmt,
						bind_index++, fi->end.REAL);
				break;
			default:
				assert(0 && "illegal datatype");
				break;
			}

			if (fmitrh->sqlite_rc != SQLITE_OK) {
				set_itr_errmsg(fmitrh, "itr_begin: bind: sqlite internal "
						"error");
				fmitrh->rc = T2MFM_SQLITE_ERROR;
				goto cleanup;
			}

			cur = cur->next;
		}
	}

	fmitrh->sqlite_rc = sqlite3_step(fmitrh->select_stmt);
	if (!(fmitrh->sqlite_rc == SQLITE_ROW || fmitrh->sqlite_rc == SQLITE_DONE))
	{
		set_itr_errmsg(fmitrh, "itr_begin: query execution failed: sqlite "
				"error");
		fmitrh->rc = T2MFM_SQLITE_ERROR;
		goto cleanup;
	}

	fmitrh->data_vec.n_elements = fmitrh->fmh->dim_meta_vec.n_elements -
			(size_t) n_dim_projected + 1;
	/* TODO: Handle out of memory */
	fmitrh->data_vec.pp_data = (void **) calloc(fmitrh->data_vec.n_elements,
			sizeof(void *));
	for (i = 0; i < fmitrh->data_vec.n_elements; i++)
		/* TODO: Handle out of memory */
		fmitrh->data_vec.pp_data[i] = calloc(1, sizeof(t2mfm_dim_data));

	*p_dim_data_vec = fmitrh->data_vec;

	fmitrh->meta_vec.n_elements = fmitrh->fmh->dim_meta_vec.n_elements -
			(size_t) n_dim_projected;
	/* TODO: Handle out of memory */
	fmitrh->meta_vec.pp_data = (void **) calloc(fmitrh->meta_vec.n_elements,
			sizeof(void *));
	for (i = 0, j = 0; i < fmitrh->projection_info_vec.n_elements; i++)
			if (((t2mfm_bool)fmitrh->projection_info_vec.pp_data[i]) ==
					T2MFM_FALSE)
				fmitrh->meta_vec.pp_data[j++] =
						fmitrh->fmh->dim_meta_vec.pp_data[i];

cleanup:
	if (fmitrh->rc != T2MFM_OK) {
		if (fmitrh->select_stmt) {
			sqlite3_finalize(fmitrh->select_stmt);
			fmitrh->select_stmt = NULL;
		}
	}

	return fmitrh->rc;
}

int t2mfm_itr_create_new_fm(t2mfm_itr *fmitrh, const char *new_matrix_name,
		t2mfm** p_new_fmh)
{
	/* TODO */
	return T2MFM_NOT_IMPLEMENTED;
}

int t2mfm_itr_next(t2mfm_itr *fmitrh)
{
	int i;
	int rc;

	if (!fmitrh)
		return T2MFM_EINVAL;

	if (!(fmitrh->sqlite_rc == SQLITE_ROW || fmitrh->sqlite_rc == SQLITE_DONE))
	{
		set_itr_errmsg(fmitrh, "itr_next: query execution failed: sqlite "
				"error");
		return (fmitrh->rc = T2MFM_SQLITE_ERROR);
	}

	if (fmitrh->sqlite_rc == SQLITE_DONE)
		return T2MFM_NO_MORE_ROWS;
	else if (fmitrh->sqlite_rc == SQLITE_ROW)
		rc = T2MFM_NEW_ROW;
	else {
		set_itr_errmsg(fmitrh, "itr_next: query execution failed: sqlite "
				"error");
		return (fmitrh->rc = T2MFM_SQLITE_ERROR);
	}

	for (i = 0; i < fmitrh->meta_vec.n_elements; i++) {
		switch (((t2mfm_dim_meta *) fmitrh->meta_vec.pp_data[i])->datatype)
		{
		case T2MFM_INT1:
			((t2mfm_dim_data *) fmitrh->data_vec.pp_data[i])->INT1 =
					(int8_t) sqlite3_column_int(fmitrh->select_stmt, i);
			break;
		case T2MFM_INT4:
			((t2mfm_dim_data *) fmitrh->data_vec.pp_data[i])->INT4 =
					sqlite3_column_int(fmitrh->select_stmt, i);
			break;
		case T2MFM_INT8:
			((t2mfm_dim_data *) fmitrh->data_vec.pp_data[i])->INT8 =
					sqlite3_column_int64(fmitrh->select_stmt, i);
			break;
		case T2MFM_REAL:
			((t2mfm_dim_data *) fmitrh->data_vec.pp_data[i])->REAL =
					sqlite3_column_double(fmitrh->select_stmt, i);
			break;
		default:
			assert(0 && "illegal datatype");
			break;
		}
	}

	((t2mfm_dim_data *) fmitrh->data_vec.pp_data[i])->INT8 =
			sqlite3_column_int(fmitrh->select_stmt, i);

	fmitrh->sqlite_rc = sqlite3_step(fmitrh->select_stmt);

	return rc;
}

char *t2mfm_itr_strerror(t2mfm_itr *fmitrh)
{
	if (!fmitrh)
		return NULL;

	return fmitrh->sqlite_errmsg;
}
