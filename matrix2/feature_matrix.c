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

#include "feature_matrix.h"
#include "feature_matrix_utils.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum states
{
	T2MFM_STATE_RAW = 1,
	T2MFM_STATE_CREATE,
	T2MFM_STATE_ADD_DIM_META_BEGIN,
	T2MFM_STATE_ADD_DIM_META_END,
	T2MFM_STATE_INSERT_BEGIN,
	T2MFM_STATE_INSERT_END,
	T2MFM_STATE_ITERATE,

	T2MFM_STATE_MAX
} states;

typedef enum stmt_type
{
	T2MFM_STMT_INSERT,
	T2MFM_STMT_UPDATE,
	T2MFM_STMT_SELECT,

	T2MFM_STMT_TYPE_MAX
} stmt_type;

static inline t2mfm *alloc_fmh();
static t2mfm_bool check_table_exists(t2mfm *fmh, const char *table_name);
static inline t2mfm_dim_meta *copy_dim_meta(const t2mfm_dim_meta *p_dim_meta);
static inline char *get_create_col(t2mfm_dim_meta *dim_meta);
static void populate_dim_meta_vec(t2mfm *fmh);
static void create_matrix(t2mfm *fmh);
static void create_indexes(t2mfm *fmh);
static inline void create_dim_data_vec(t2mfm *fmh);
static void bind_dim_parameters(t2mfm *fmh, stmt_type stmt_type);

extern t2mfm_itr *alloc_fmitrh(t2mfm *fmh);
extern void free_fmitrh(t2mfm_itr *fmitrh);

static inline t2mfm *alloc_fmh()
{
	t2mfm *fmh = (t2mfm *) calloc(1, sizeof(t2mfm));
	if (!fmh)
		return NULL;

	fmh->rc = T2MFM_OK;
	fmh->sqlite_rc = SQLITE_OK;
	fmh->state = T2MFM_STATE_RAW;

	return fmh;
}

/* @return: Non zero if the table exists, zero if the table does not exist */
static t2mfm_bool check_table_exists(t2mfm *fmh, const char *table_name)
{
	t2mfm_bool table_found = T2MFM_FALSE;
	char sqlbuf[4096];
	sqlite3_stmt *stmt = NULL;

	if (!fmh || !fmh->db || !table_name)
		return 0;

	/* TODO: Validate table name length. */
	snprintf(sqlbuf, sizeof(sqlbuf), "select * from sqlite_master where "
			"tbl_name = '%s';", table_name);

	fmh->sqlite_rc = sqlite3_prepare_v2(fmh->db, sqlbuf, -1, &stmt, NULL);
	if (fmh->sqlite_rc != SQLITE_OK)
		goto cleanup;

	if (sqlite3_step(stmt) == SQLITE_ROW)
		table_found = T2MFM_TRUE;

cleanup:
	if (stmt)
		sqlite3_finalize(stmt);

	return table_found;
}

static inline t2mfm_dim_meta *copy_dim_meta(const t2mfm_dim_meta *p_dim_meta)
{
	t2mfm_dim_meta *p_dim_meta_copy;
	static int ordinal = 0;

	if (!p_dim_meta)
		return NULL;

	p_dim_meta_copy = (t2mfm_dim_meta *) calloc(1, sizeof(t2mfm_dim_meta));
	if (!p_dim_meta_copy)
		return NULL;

	p_dim_meta_copy->ordinal = ordinal++;
	p_dim_meta_copy->name = strdup(p_dim_meta->name);
	p_dim_meta_copy->desc = strdup(p_dim_meta->desc);
	p_dim_meta_copy->datatype = p_dim_meta->datatype;
	p_dim_meta_copy->is_indexed = p_dim_meta->is_indexed;

	return p_dim_meta_copy;
}

static inline char *get_create_col(t2mfm_dim_meta *dim_meta)
{
	static char buf[1024];
	char *datatype_str = NULL;

	assert(dim_meta);
	switch (dim_meta->datatype) {
	case T2MFM_INT1:
	case T2MFM_INT4:
	case T2MFM_INT8:
		datatype_str = "integer";
		break;
	case T2MFM_REAL:
		datatype_str = "real";
		break;
	default:
		assert(!"illegal datatype");
		break;
	}

	snprintf(buf, sizeof(buf) - 1, "%s %s", dim_meta->name, datatype_str);
	return buf;
}

static void populate_dim_meta_vec(t2mfm *fmh)
{
	assert(fmh);
	t2mfm_dll_node *cur;
	int i;

	if (!fmh->dim_meta_head)
		return;

	fmh->dim_meta_vec.n_elements = list_size(fmh->dim_meta_head);
	fmh->dim_meta_vec.pp_data = (void **) calloc(fmh->dim_meta_vec.n_elements,
			sizeof(void *)); /* TODO: Handle out of memory */

	i = 0;
	cur = fmh->dim_meta_head;
	while (cur) {
		fmh->dim_meta_vec.pp_data[i++] = cur->data;
		cur = cur->next;
	}

	return;
}

static void create_metadata_table(t2mfm *fmh)
{
	const char *sql = "create table t2mfm_master ("
			"matrix_name text, "
			"dim_ordinal integer, "
			"dim_name text, "
			"dim_desc text, "
			"dim_datatype integer, "
			"dim_is_indexed integer, "
			"primary key (matrix_name, dim_name));";

	assert(fmh);

	fmh->sqlite_rc = sqlite3_exec(fmh->db, sql, NULL, NULL, NULL);
	if (fmh->rc != SQLITE_OK) {
		set_errmsg(fmh, "create table t2mfm_master failed");
		fmh->rc = T2MFM_SQLITE_ERROR;
		return;
	}

	return;
}

#define HANDLE_BIND_RC(fmh, label) \
	if (fmh->sqlite_rc != SQLITE_OK) { \
		set_errmsg(fmh, "popluate_metadata bind error"); \
		fmh->rc = T2MFM_SQLITE_ERROR; \
		goto label; \
	}

static void populate_metadata(t2mfm *fmh)
{
	const char *sql = "insert into t2mfm_master (matrix_name, dim_ordinal, "
			"dim_name, dim_desc, dim_datatype, dim_is_indexed) values "
			"(?, ?, ?, ?, ?, ?);";
	sqlite3_stmt *stmt = NULL;
	t2mfm_vec *meta_vec;
	int i;

	assert(fmh);

	fmh->sqlite_rc = sqlite3_prepare_v2(fmh->db, sql, -1, &stmt,  NULL);
	if (fmh->sqlite_rc != T2MFM_OK) {
		set_errmsg(fmh, "internal error: populate_metadata: sqlite prepare"
				" failed");
		fmh->rc = T2MFM_SQLITE_ERROR;
		return;
	}

	meta_vec = &fmh->dim_meta_vec;
	for (i = 0; i < meta_vec->n_elements; i++) {
		t2mfm_dim_meta *dim_meta = (t2mfm_dim_meta *) meta_vec->pp_data[i];
		fmh->sqlite_rc = sqlite3_bind_text(stmt, 1, fmh->matrix_name, -1, NULL);
		HANDLE_BIND_RC(fmh, cleanup);

		fmh->sqlite_rc = sqlite3_bind_int(stmt, 2, dim_meta->ordinal);
		HANDLE_BIND_RC(fmh, cleanup);

		fmh->sqlite_rc = sqlite3_bind_text(stmt, 3, dim_meta->name, -1, NULL);
		HANDLE_BIND_RC(fmh, cleanup);

		fmh->sqlite_rc = sqlite3_bind_text(stmt, 4, dim_meta->desc, -1, NULL);
		HANDLE_BIND_RC(fmh, cleanup);

		fmh->sqlite_rc = sqlite3_bind_int(stmt, 5, dim_meta->datatype);
		HANDLE_BIND_RC(fmh, cleanup);

		fmh->sqlite_rc = sqlite3_bind_int(stmt, 6, dim_meta->is_indexed);
		HANDLE_BIND_RC(fmh, cleanup);

		fmh->sqlite_rc = sqlite3_step(stmt);
		if (fmh->sqlite_rc != SQLITE_DONE) {
			set_errmsg(fmh, "step failed");
			fmh->rc = T2MFM_SQLITE_ERROR;
			goto cleanup;
		}

		fmh->sqlite_rc = sqlite3_clear_bindings(stmt);
		if (fmh->sqlite_rc != SQLITE_OK) {
			set_errmsg(fmh, "internal error: populate_metadata: sqlite clear bindings "
					"failed");
			fmh->rc = T2MFM_SQLITE_ERROR;
			goto cleanup;
		}

		fmh->sqlite_rc = sqlite3_reset(stmt);
		if (fmh->sqlite_rc != SQLITE_OK) {
			set_errmsg(fmh, "internal error: populate_metadata: sqlite reset "
					"failed");
			fmh->rc = T2MFM_SQLITE_ERROR;
			goto cleanup;
		}
	}

cleanup:
	sqlite3_finalize(stmt);
	return;
}

static void create_matrix_table(t2mfm *fmh)
{
	const size_t sqlbuf_size = 4096;
	char sqlbuf[sqlbuf_size];
	int chars_written = 0;
	t2mfm_vec *dim_vec;
	int i;

	assert(fmh);

	chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size -
			(size_t) chars_written, "create table %s (", fmh->matrix_name);

	dim_vec = &fmh->dim_meta_vec;
	for (i = 0; i < dim_vec->n_elements && chars_written < sqlbuf_size; i++)
		chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size - (size_t)
				chars_written, "%s, ", get_create_col(dim_vec->pp_data[i]));

	if (chars_written < sqlbuf_size)
		chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size - (size_t)
				chars_written, "t2mfm_frequency integer);");

	fmh->sqlite_rc = sqlite3_exec(fmh->db, sqlbuf , NULL, NULL, NULL);
	if (fmh->sqlite_rc != SQLITE_OK) {
		set_errmsg(fmh, "internal error");
		fmh->rc = T2MFM_SQLITE_ERROR;
		return;
	}

	return;
}

static void create_matrix(t2mfm *fmh)
{
	t2mfm_bool t2mfm_master_table;

	assert(fmh);

	/* Check if meta-data table exists */
	t2mfm_master_table = check_table_exists(fmh, "t2mfm_master");
	if (fmh->rc != T2MFM_OK)
		return;

	if (t2mfm_master_table == T2MFM_FALSE) {
		create_metadata_table(fmh);
		if (fmh->rc != T2MFM_OK)
			return;
	}

	populate_metadata(fmh);
	if (fmh->rc != T2MFM_OK)
		return;

	create_matrix_table(fmh);

	return;
}

static void create_indexes(t2mfm *fmh)
{
	int i;
	t2mfm_vec *dim_vec;
	char sqlbuf[4096];
	int pos = 0;

	assert(fmh);

	dim_vec = &fmh->dim_meta_vec;
	for (i = 0; i < dim_vec->n_elements; i++) {
		t2mfm_dim_meta *dim_meta = dim_vec->pp_data[i];
		if (dim_meta->is_indexed) {

			if (!i) {
				pos = snprintf(sqlbuf, sizeof(sqlbuf),
					"create index if not exists"
					" idx_%s on %s", fmh->matrix_name,
					fmh->matrix_name);
				pos += snprintf(sqlbuf + pos,
					sizeof(sqlbuf) - pos,
					"(%s", dim_meta->name);
			} else
				pos += snprintf(sqlbuf + pos,
					sizeof(sqlbuf) - pos,
					",%s", dim_meta->name);
		}
	}

	/* none of the dimensions were marked for indexing */
	if (!pos)
		return;

	pos += snprintf(sqlbuf + pos, sizeof(sqlbuf), ");");

	fmh->sqlite_rc = sqlite3_exec(fmh->db, sqlbuf, NULL, NULL, NULL);
	if (fmh->sqlite_rc != SQLITE_OK) {
		set_errmsg(fmh, "error creating index");
		fmh->rc = T2MFM_SQLITE_ERROR;
	}
}

static inline void create_dim_data_vec(t2mfm *fmh)
{
	int i;

	assert(fmh);

	fmh->dim_data_vec.n_elements = fmh->dim_meta_vec.n_elements;
	fmh->dim_data_vec.pp_data = (void **) calloc(fmh->dim_data_vec.n_elements,
			sizeof(void *)); /* TODO: Handle out of memory */
	for (i = 0; i < fmh->dim_data_vec.n_elements; i++)
		/* TODO: Handle out of memory */
		fmh->dim_data_vec.pp_data[i] = calloc(1, sizeof(t2mfm_dim_data));

	return;
}

static void bind_dim_parameters(t2mfm *fmh, stmt_type stmt_type)
{
	int i;
	t2mfm_vec *meta_vec, *data_vec;
	sqlite3_stmt *stmt;

	assert(fmh);
	assert(stmt_type >= 0 && stmt_type < T2MFM_STMT_TYPE_MAX);

	switch (stmt_type) {
	case T2MFM_STMT_INSERT:
		stmt = fmh->insert_stmt;
		break;
	case T2MFM_STMT_UPDATE:
		stmt = fmh->update_stmt;
		break;
	case T2MFM_STMT_SELECT:
		stmt = fmh->select_stmt;
		break;
	default:
		assert(0 && "Illegal statement type");
		break;
	}

	fmh->sqlite_rc = sqlite3_clear_bindings(stmt);
	if (fmh->sqlite_rc != SQLITE_OK) {
		set_errmsg(fmh, "clearing failed");
		fmh->rc = T2MFM_SQLITE_ERROR;
		return;
	}

	fmh->sqlite_rc = sqlite3_reset(stmt);
	if (fmh->sqlite_rc != SQLITE_OK) {
		set_errmsg(fmh, "reset failed");
		fmh->rc = T2MFM_SQLITE_ERROR;
		return;
	}

	meta_vec = &fmh->dim_meta_vec;
	data_vec = &fmh->dim_data_vec;
	for (i = 0; i < data_vec->n_elements; i++) {
		t2mfm_dim_meta *dim_meta = (t2mfm_dim_meta *) meta_vec->pp_data[i];
		t2mfm_dim_data *dim_data = (t2mfm_dim_data *) data_vec->pp_data[i];
		switch (dim_meta->datatype) {
		case T2MFM_INT1:
			fmh->sqlite_rc = sqlite3_bind_int(stmt, i + 1,
					dim_data->INT1);
			break;
		case T2MFM_INT4:
			fmh->sqlite_rc = sqlite3_bind_int(stmt, i + 1,
					dim_data->INT4);
			break;
		case T2MFM_INT8:
			fmh->sqlite_rc = sqlite3_bind_int64(stmt, i + 1,
					dim_data->INT8);
			break;
		case T2MFM_REAL:
			fmh->sqlite_rc = sqlite3_bind_double(stmt, i + 1,
					dim_data->REAL);
			break;
		default:
			assert(!"invalid datatype");
			break;
		}

		if (fmh->sqlite_rc != SQLITE_OK) {
			set_errmsg(fmh, "binding failed");
			fmh->rc = T2MFM_SQLITE_ERROR;
			return;
		}
	}

	if (stmt_type == T2MFM_STMT_INSERT) {
		fmh->sqlite_rc = sqlite3_bind_int(stmt, i + 1, 1);
		if (fmh->sqlite_rc != SQLITE_OK) {
			set_errmsg(fmh, "binding failed");
			fmh->rc = T2MFM_SQLITE_ERROR;
			return;
		}
	}

	return;
}

static void load_dim_meta(t2mfm *fmh)
{
	const char *sql = "select dim_ordinal, dim_name, dim_desc, dim_datatype, "
			"dim_is_indexed from t2mfm_master where matrix_name = ? order by "
			"dim_ordinal;";
	sqlite3_stmt *stmt = NULL;

	assert(fmh);

	fmh->sqlite_rc = sqlite3_prepare_v2(fmh->db, sql, -1, &stmt, NULL);
	if (fmh->sqlite_rc) {
		set_errmsg(fmh, "load_dim_meta: prepare failed");
		fmh->rc = T2MFM_SQLITE_ERROR;
		goto cleanup;
	}

	fmh->sqlite_rc = sqlite3_bind_text(stmt, 1, fmh->matrix_name, -1, NULL);
	if (fmh->sqlite_rc) {
		set_errmsg(fmh, "load_dim_meta: bind failed");
		fmh->rc = T2MFM_SQLITE_ERROR;
		goto cleanup;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		t2mfm_dim_meta *dim_meta = (t2mfm_dim_meta *) calloc(1,
				sizeof(t2mfm_dim_meta)); /* TODO: Handle out of memory */
		dim_meta->ordinal = sqlite3_column_int(stmt, 0);
		dim_meta->name = strdup((char *) sqlite3_column_text(stmt, 1));
		dim_meta->desc = strdup((char *) sqlite3_column_text(stmt, 2));
		dim_meta->datatype = sqlite3_column_int(stmt, 3);
		dim_meta->is_indexed = sqlite3_column_int(stmt, 4);

		fmh->dim_meta_head = list_add_tail(fmh->dim_meta_head, dim_meta);
	}

	populate_dim_meta_vec(fmh);
	if (fmh->rc != T2MFM_OK)
		goto cleanup;

 cleanup:
	 if (stmt)
		 sqlite3_finalize(stmt);

	 return;
}

static void get_insert_stmt(t2mfm *fmh, char *sqlbuf, size_t sqlbuf_size)
{
	t2mfm_vec *dim_vec;
	int chars_written = 0;
	int i;

	assert(fmh);
	assert(sqlbuf);

	/* insert into */
	chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size - (size_t)
			chars_written, "insert into %s (", fmh->matrix_name);

	/* column names */
	dim_vec = &fmh->dim_meta_vec;
	for (i = 0; i < dim_vec->n_elements && chars_written < sqlbuf_size ; i++)
		chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size - (size_t)
				chars_written, "%s, ",
				((t2mfm_dim_meta *) dim_vec->pp_data[i])->name);

	/* values */
	if (chars_written < sqlbuf_size) {
		chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size - (size_t)
				chars_written, "t2mfm_frequency) values (");
	}

	/* '?' (bind place holder) */
	for (i = 0; i < dim_vec->n_elements + 1 && chars_written < sqlbuf_size; i++)
		chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size - (size_t)
				chars_written, i < dim_vec->n_elements? "?, " : "?);");

	return;
}

static void get_select_stmt(t2mfm *fmh, char *sqlbuf, size_t sqlbuf_size)
{
	t2mfm_vec *dim_vec;
	int chars_written = 0;
	int i;

	assert(fmh);
	assert(sqlbuf);

	/* select */
	chars_written += snprintf(sqlbuf, sqlbuf_size, "select ");

	/* all columns */
	dim_vec = &fmh->dim_meta_vec;
	for (i = 0; i < dim_vec->n_elements && chars_written < sqlbuf_size; i++)
		chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size - (size_t)
				chars_written, i < dim_vec->n_elements - 1 ? "%s, " : "%s ",
						((t2mfm_dim_meta *) dim_vec->pp_data[i])->name);

	/* from table where */
	if (chars_written < sqlbuf_size)
		chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size - (size_t)
				chars_written, "from %s where ", fmh->matrix_name);

	/* column1 = '?' and . . . */
	for (i = 0; i < dim_vec->n_elements && chars_written < sqlbuf_size ; i++)
		chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size - (size_t)
				chars_written, i < dim_vec->n_elements - 1 ? "%s = ? and " :
				"%s = ?;", ((t2mfm_dim_meta *) dim_vec->pp_data[i])->name);

	return;
}

static void get_update_stmt(t2mfm *fmh, char *sqlbuf, size_t sqlbuf_size)
{
	t2mfm_vec *dim_vec;
	int chars_written = 0;
	int i;

	assert(fmh);
	assert(sqlbuf);

	dim_vec = &fmh->dim_meta_vec;

	/* update table set t2mfm_frequency = t2mfm_frequency + 1 where */
	chars_written += snprintf(sqlbuf, sqlbuf_size, "update %s set "
			"t2mfm_frequency = t2mfm_frequency + 1 where ", fmh->matrix_name);

	/* column1 = '?' and . . . */
	for (i = 0; i < dim_vec->n_elements && chars_written < sqlbuf_size ; i++)
		chars_written += snprintf(sqlbuf + chars_written, sqlbuf_size - (size_t)
				chars_written, i < dim_vec->n_elements - 1 ? "%s = ? and " :
				"%s = ?;", ((t2mfm_dim_meta *) dim_vec->pp_data[i])->name);

	return;
}

static int disable_sync_and_journal(t2mfm *fmh)
{
	int ret = T2MFM_OK;

	fmh->sqlite_rc = sqlite3_exec(fmh->db,
				"PRAGMA synchronous = OFF;",
				NULL, NULL, NULL);
	if (fmh->sqlite_rc != SQLITE_OK) {
		set_errmsg(fmh, "disabling synchronous mode failed");
		ret = T2MFM_SQLITE_ERROR;
	}

	fmh->sqlite_rc = sqlite3_exec(fmh->db,
			"PRAGMA journal_mode = OFF",
			NULL, NULL, NULL);
	if (fmh->sqlite_rc != SQLITE_OK) {
		set_errmsg(fmh, "disabling journal failed");
		ret = T2MFM_SQLITE_ERROR;
	}

	return ret;
}

int t2mfm_open(const char *backing_store_uri, const char *matrix_name,
		t2mfm_open_mode open_mode, t2mfm **p_fmh)
{
	int sqlite_open_flags = 0;
	t2mfm_bool table_exists;

	if (!backing_store_uri || !strlen(backing_store_uri))
		return T2MFM_EINVAL;

	if (!matrix_name || !strlen(matrix_name))
		return T2MFM_EINVAL;

	if (open_mode < 0 || open_mode > T2MFM_OPEN_MAX)
		return T2MFM_EINVAL;

	if (!p_fmh)
		return T2MFM_EINVAL;

	*p_fmh = alloc_fmh();
	if (!*p_fmh)
		return T2MFM_ENOMEM;

	/* TODO: Handle out of memory for strdup */
	(*p_fmh)->backing_store_uri = strdup(backing_store_uri);
	(*p_fmh)->matrix_name = strdup(matrix_name);
	(*p_fmh)->open_mode = open_mode;

	switch ((*p_fmh)->open_mode) {
	case T2MFM_OPEN_CREATE:
		sqlite_open_flags = SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE;
			
		break;
	case T2MFM_OPEN_ITERATE:
		sqlite_open_flags = SQLITE_OPEN_READONLY;
		break;
	}

	(*p_fmh)->sqlite_rc = sqlite3_open_v2((*p_fmh)->backing_store_uri,
			&((*p_fmh)->db), sqlite_open_flags, NULL);
	if ((*p_fmh)->sqlite_rc != SQLITE_OK) {
		set_errmsg(*p_fmh, "sqlite open failed");
		(*p_fmh)->rc = T2MFM_SQLITE_ERROR;
		goto cleanup;
	}

	(*p_fmh)->rc = disable_sync_and_journal(*p_fmh);
	if ((*p_fmh)->rc != T2MFM_OK)
		goto cleanup;

	table_exists = check_table_exists(*p_fmh, (*p_fmh)->matrix_name);
	switch ((*p_fmh)->open_mode) {
	case T2MFM_OPEN_CREATE:
		if (table_exists == T2MFM_TRUE) {
			set_errmsg(*p_fmh, "matrix exists");
			(*p_fmh)->rc = T2MFM_MATRIX_EXISTS;
			goto cleanup;
		}

		(*p_fmh)->state = T2MFM_STATE_CREATE;
		break;
	case T2MFM_OPEN_ITERATE:
		if (table_exists == T2MFM_FALSE) {
			set_errmsg(*p_fmh, "missing matrix");
			(*p_fmh)->rc = T2MFM_MATRIX_MISSING;
			goto cleanup;
		}

		load_dim_meta(*p_fmh);
		if ((*p_fmh)->rc != T2MFM_OK)
			goto cleanup;

		(*p_fmh)->state = T2MFM_STATE_ITERATE;
		break;
	}

	(*p_fmh)->rc = T2MFM_OK;

cleanup:
	if ((*p_fmh)->rc != T2MFM_OK && (*p_fmh)->db)
		sqlite3_close((*p_fmh)->db);

	return (*p_fmh)->rc;
}

int t2mfm_add_dim_meta_begin(t2mfm *fmh)
{
	if (!fmh)
		return T2MFM_EINVAL;

	if (fmh->state != T2MFM_STATE_CREATE) {
		set_errmsg(fmh, "invalid mode");
		return (fmh->rc = T2MFM_EINVAL_MODE);
	}

	fmh->dim_meta_head = NULL;
	fmh->state = T2MFM_STATE_ADD_DIM_META_BEGIN;
	return (fmh->rc = T2MFM_OK);
}

int t2mfm_add_dim_meta(t2mfm *fmh, const t2mfm_dim_meta *p_dim_meta)
{
	if (!fmh)
		return T2MFM_EINVAL;

	if (fmh->state != T2MFM_STATE_ADD_DIM_META_BEGIN) {
		set_errmsg(fmh, "invalid state: t2mfm_add_dim_meta_begin not"
				" invoked");
		return (fmh->rc = T2MFM_EINVAL_STATE);
	}

	if (!p_dim_meta)
		return T2MFM_EINVAL;

	if (!strncmp("t2mfm", p_dim_meta->name, strlen("t2mfm"))) {
		set_errmsg(fmh, "dimension has reserved name");
		return (fmh->rc = T2MFM_EINVAL);
	}

	fmh->dim_meta_head = list_add_tail(fmh->dim_meta_head,
			copy_dim_meta(p_dim_meta));

	return (fmh->rc = T2MFM_OK);
}

void t2mfm_add_dim_meta_end(t2mfm *fmh)
{
	if (!fmh || fmh->state != T2MFM_STATE_ADD_DIM_META_BEGIN)
		return;

	/* At least one dimension must exist */
	if (!fmh->dim_meta_head)
		return;

	assert(fmh->db && "missing connection to sqlite");

	populate_dim_meta_vec(fmh);
	if (fmh->rc != T2MFM_OK)
		return;

	create_matrix(fmh);
	if (fmh->rc != T2MFM_OK)
		return;

	fmh->state = T2MFM_STATE_ADD_DIM_META_END;
	return;
}

int t2mfm_get_dim_meta(t2mfm *fmh, t2mfm_vec *p_dim_meta_vec)
{
	if (!fmh)
		return T2MFM_EINVAL;

	if (!p_dim_meta_vec) {
		set_errmsg(fmh, "NULL p_dim_meta_vec");
		return (fmh->rc = T2MFM_EINVAL);
	}

	if (fmh->state != T2MFM_STATE_ITERATE) {
		set_errmsg(fmh, "invalid state");
		return (fmh->rc = T2MFM_EINVAL_STATE);
	}

	*p_dim_meta_vec = fmh->dim_meta_vec;

	return fmh->rc;
}

int t2mfm_insert_begin(t2mfm *fmh, t2mfm_vec *p_dim_data_vec)
{
	const size_t sqlbuf_size = 4096;
	char sqlbuf[sqlbuf_size];

	if (!fmh)
		return T2MFM_EINVAL;

	if (fmh->state != T2MFM_STATE_ADD_DIM_META_END) {
		set_errmsg(fmh, "invalid state: t2mfm_add_dim_meta_end not complete");
		return (fmh->rc = T2MFM_EINVAL_STATE);
	}

	if (!p_dim_data_vec) {
		set_errmsg(fmh, "NULL p_dim_data_vec");
		return (fmh->rc = T2MFM_EINVAL);
	}

	get_insert_stmt(fmh, sqlbuf, sqlbuf_size);
	if (fmh->rc != T2MFM_OK)
		goto out;

	fmh->sqlite_rc = sqlite3_prepare_v2(fmh->db, sqlbuf, -1, &fmh->insert_stmt,
			NULL);
	if (fmh->sqlite_rc != SQLITE_OK)  {
		set_errmsg(fmh, "unable to prepare statement for insert");
		fmh->rc = T2MFM_SQLITE_ERROR;
		goto out;
	}

	get_update_stmt(fmh, sqlbuf, sqlbuf_size);
	if (fmh->rc != T2MFM_OK)
		goto out;

	fmh->sqlite_rc = sqlite3_prepare_v2(fmh->db, sqlbuf, -1, &fmh->update_stmt,
			NULL);
	if (fmh->sqlite_rc != SQLITE_OK)  {
		set_errmsg(fmh, "unable to prepare statement for update");
		fmh->rc = T2MFM_SQLITE_ERROR;
		goto out;
	}

	get_select_stmt(fmh, sqlbuf, sqlbuf_size);
	if (fmh->rc != SQLITE_OK)
		goto out;

	fmh->sqlite_rc = sqlite3_prepare_v2(fmh->db, sqlbuf, -1, &fmh->select_stmt,
			NULL);
	if (fmh->sqlite_rc != SQLITE_OK)  {
		set_errmsg(fmh, "unable to prepare statement for select");
		fmh->rc = T2MFM_SQLITE_ERROR;
		goto out;
	}

	create_indexes(fmh);
	if (fmh->rc != T2MFM_OK)
		goto out;

	create_dim_data_vec(fmh);
	*p_dim_data_vec = fmh->dim_data_vec;

	fmh->state = T2MFM_STATE_INSERT_BEGIN;

out:
	if (fmh->rc != T2MFM_OK) {
		if (fmh->insert_stmt) {
			sqlite3_finalize(fmh->insert_stmt);
			fmh->insert_stmt = NULL;
		}
		if (fmh->update_stmt) {
			sqlite3_finalize(fmh->update_stmt);
			fmh->update_stmt = NULL;
		}
		if (fmh->select_stmt) {
			sqlite3_finalize(fmh->select_stmt);
			fmh->select_stmt = NULL;
		}
	}

	return fmh->rc;
}

int t2mfm_insert(t2mfm *fmh)
{
	if (!fmh)
		return T2MFM_EINVAL;

	if (fmh->state != T2MFM_STATE_INSERT_BEGIN) {
		set_errmsg(fmh, "invalid state: t2mfm_insert_begin not complete");
		return (fmh->rc = T2MFM_EINVAL_STATE);
	}

	bind_dim_parameters(fmh, T2MFM_STMT_SELECT);
	if (fmh->rc != T2MFM_OK)
		goto out;

	fmh->sqlite_rc = sqlite3_step(fmh->select_stmt);
	switch (fmh->sqlite_rc) {
	case SQLITE_ROW:
		bind_dim_parameters(fmh, T2MFM_STMT_UPDATE);
		if (fmh->rc != T2MFM_OK)
			goto out;

		fmh->sqlite_rc = sqlite3_step(fmh->update_stmt);
		if (fmh->sqlite_rc != SQLITE_DONE) {
			set_errmsg(fmh, "internal error: sqlite update error");
			fmh->rc = T2MFM_SQLITE_ERROR;
			goto out;
		}
		break;
	case SQLITE_DONE:
		bind_dim_parameters(fmh, T2MFM_STMT_INSERT);
		if (fmh->rc != T2MFM_OK)
			goto out;

		fmh->sqlite_rc = sqlite3_step(fmh->insert_stmt);
		if (fmh->sqlite_rc != SQLITE_DONE) {
			fmh->rc = T2MFM_SQLITE_ERROR;
			goto out;
		}
		break;
	default:
		fmh->rc = T2MFM_SQLITE_ERROR;
		goto out;
		break;
	}

out:
	return fmh->rc;
}

void t2mfm_insert_end(t2mfm *fmh)
{
	if (!fmh || fmh->state != T2MFM_STATE_INSERT_BEGIN)
		return;

	fmh->sqlite_rc = sqlite3_finalize(fmh->insert_stmt);
	if (fmh->sqlite_rc != SQLITE_OK) {
		set_errmsg(fmh, "internal error: sqlite insert statement finalization "
				"failed");
		fmh->rc = T2MFM_SQLITE_ERROR;
		return;
	}
	fmh->insert_stmt = NULL;

	fmh->sqlite_rc = sqlite3_finalize(fmh->update_stmt);
	if (fmh->sqlite_rc != SQLITE_OK) {
		set_errmsg(fmh, "internal error: sqlite update statement finalization "
				"failed");
		fmh->rc = T2MFM_SQLITE_ERROR;
		return;
	}
	fmh->update_stmt = NULL;

	fmh->sqlite_rc = sqlite3_finalize(fmh->select_stmt);
	if (fmh->sqlite_rc != SQLITE_OK) {
		set_errmsg(fmh, "insert error: sqlite select statement finalization "
				"failed");
		fmh->rc = T2MFM_SQLITE_ERROR;
		return;
	}
	fmh->select_stmt = NULL;

	fmh->sqlite_rc = sqlite3_close(fmh->db);
	if (fmh->sqlite_rc != SQLITE_OK) {
		fmh->db = NULL;
		set_errmsg(fmh, "close failed");
		fmh->rc = T2MFM_SQLITE_ERROR;
		return;
	}

	fmh->sqlite_rc = sqlite3_open_v2(fmh->backing_store_uri, &fmh->db,
			SQLITE_OPEN_READONLY, NULL);
	if (fmh->sqlite_rc != SQLITE_OK) {
		fmh->db = NULL;
		set_errmsg(fmh, "opening a read only connection failed");
		fmh->rc = T2MFM_SQLITE_ERROR;
		return;
	}

	fmh->state = T2MFM_STATE_ITERATE; /* Not T2MFM_STATE_INSERT_END */
	return;
}

int t2mfm_itr_alloc(t2mfm *fmh, t2mfm_itr **p_fmitrh)
{
	t2mfm_itr *fmitrh;

	if (!fmh)
		return T2MFM_EINVAL;

	if (fmh->state != T2MFM_STATE_ITERATE) {
		set_errmsg(fmh, "invalid state");
		return (fmh->rc = T2MFM_EINVAL_STATE);
	}

	if (!p_fmitrh) {
		set_errmsg(fmh, "NULL p_fmirth");
		return (fmh->rc = T2MFM_EINVAL_STATE);
	}

	fmitrh = alloc_fmitrh(fmh);
	if (fmh->rc == T2MFM_OK && fmitrh)
		*p_fmitrh = fmitrh;

	return fmh->rc;
}

void t2mfm_itr_free(t2mfm_itr *fmitrh)
{
	free_fmitrh(fmitrh);
}

int t2mfm_get_stat(t2mfm *fmh, int dim_ordinal, t2mfm_stat_type stat_type,
		t2mfm_dim_data *stat_val)
{
	char sqlbuf[1024];
	char *grouping_fn_name;
	sqlite3_stmt *stmt = NULL;

	if (!fmh)
		return T2MFM_EINVAL;

	if (fmh->state != T2MFM_STATE_ITERATE) {
		set_errmsg(fmh, "invalid state");
		return (fmh->rc = T2MFM_EINVAL_STATE);
	}

	if (dim_ordinal < 0 || dim_ordinal > fmh->dim_meta_vec.n_elements - 1) {
		set_errmsg(fmh, "invalid ordinal");
		return (fmh->rc = T2MFM_EINVAL);
	}

	if (stat_type < 0 || stat_type >= T2MFM_STAT_END) {
		set_errmsg(fmh, "invalid stat type");
		return (fmh->rc = T2MFM_EINVAL);
	}

	if (!stat_val) {
		set_errmsg(fmh, "NULL stat_val");
		return (fmh->rc = T2MFM_EINVAL);
	}

	switch (stat_type) {
	case T2MFM_STAT_AVG:
		grouping_fn_name = "avg";
		break;
	case T2MFM_STAT_MAX:
		grouping_fn_name = "max";
		break;
	case T2MFM_STAT_MIN:
		grouping_fn_name = "min";
		break;
	case T2MFM_STAT_SUM:
		grouping_fn_name = "sum";
		break;
	case T2MFM_STAT_COUNT:
		grouping_fn_name =  "count";
		break;
	default:
		assert(!"illegal stat");
		break;
	}

	snprintf(sqlbuf, sizeof(sqlbuf), "select %s(%s) from %s;", grouping_fn_name,
			((t2mfm_dim_meta *) fmh->dim_meta_vec.pp_data[dim_ordinal])->name,
			fmh->matrix_name);

	fmh->sqlite_rc = sqlite3_prepare_v2(fmh->db, sqlbuf, -1, &stmt, NULL);
	if (fmh->sqlite_rc != SQLITE_OK) {
		set_errmsg(fmh, "aggregate query failed");
		fmh->rc = T2MFM_SQLITE_ERROR;
		goto cleanup;
	}

	fmh->sqlite_rc = sqlite3_step(stmt);
	if (fmh->sqlite_rc != SQLITE_ROW) {
		set_errmsg(fmh, "no rows found");
		fmh->rc = T2MFM_SQLITE_ERROR;
		goto cleanup;
	}

	switch (stat_type) {
	case T2MFM_STAT_AVG:
		(*stat_val).REAL = sqlite3_column_double(stmt, 0);
		break;
	case T2MFM_STAT_MIN:
	case T2MFM_STAT_MAX:
	case T2MFM_STAT_SUM:
		switch (((t2mfm_dim_meta *) fmh->dim_meta_vec.pp_data[0])
				->datatype) {
		case T2MFM_INT1:
			(*stat_val).INT1 = (int8_t) sqlite3_column_int(stmt, 0);
			break;
		case T2MFM_INT4:
			(*stat_val).INT4 = (int32_t) sqlite3_column_int(stmt, 0);
			break;
		case T2MFM_INT8:
			(*stat_val).INT8 = sqlite3_column_int64(stmt, 0);
			break;
		case T2MFM_REAL:
			(*stat_val).REAL = sqlite3_column_double(stmt, 0);
			break;
		default:
			assert(!"illegal datatype");
			break;
		}
		break;
	case T2MFM_STAT_COUNT:
		(*stat_val).INT8 = sqlite3_column_int64(stmt, 0);
		break;
	default:
		assert(!"illegal stat type");
		break;
	}

cleanup:
	if (stmt)
		sqlite3_finalize(stmt);

	return fmh->rc;
}

void t2mfm_close(t2mfm *fmh)
{
	if (!fmh)
		return;

	if (fmh->backing_store_uri)
		free(fmh->backing_store_uri);

	if (fmh->matrix_name)
		free(fmh->matrix_name);

	if (fmh->errmsg)
		free(fmh->errmsg);

	list_free(fmh->dim_meta_head, T2MFM_TRUE);
	vec_free(&fmh->dim_meta_vec, T2MFM_FALSE);
	vec_free(&fmh->dim_data_vec, T2MFM_TRUE);

	if (fmh->insert_stmt && fmh->db)
		sqlite3_finalize(fmh->insert_stmt);

	if (fmh->db)
		sqlite3_close(fmh->db);
}

int t2mfm_get_matrix_names(const char *backing_store_uri,
		t2mfm_dll_node *p_head)
{
	/* TODO */
	return T2MFM_NOT_IMPLEMENTED;
}

char *t2mfm_strerror(t2mfm *fmh)
{
	if (!fmh)
		return NULL;

	return fmh->errmsg;
}
