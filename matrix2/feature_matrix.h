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

#ifndef FEATURE_MATRIX_H_
#define FEATURE_MATRIX_H_

#include <sqlite3.h>
#include <stdlib.h>
#include <stdint.h>

/* Boolean type definitions to be used within the feature matrix API */
typedef enum t2mfm_bool
{
    T2MFM_FALSE = 0,
    T2MFM_TRUE
} t2mfm_bool;

/* Open modes */
typedef enum t2mfm_open_mode
{
    T2MFM_OPEN_CREATE,
    T2MFM_OPEN_ITERATE,

    T2MFM_OPEN_MAX
} t2mfm_open_mode;

/* A simple generic doubly linked list. TODO: Consider using linux/list.h */
typedef struct t2mfm_dll_node
{
    struct t2mfm_dll_node *prev, *next;
    void *data;
} t2mfm_dll_node;

/* An simple generic vector. TODO: Consider using something more standardized */
typedef struct t2mfm_vec
{
    size_t n_elements;
    void **pp_data;
} t2mfm_vec;

/* The Trace2Model Feature Matrix. A feature matrix handle, usually abbreviated
 * as fmh, is a pointer to this structure. */
typedef struct t2mfm
{
/* Public state */
    char *backing_store_uri;
    char *matrix_name;
    int open_mode;
    int rc;
    char *errmsg; /* Always points to memory that can be freed */

/* Private state */
    sqlite3 *db;
    int sqlite_rc;
    /* This message is mostly relevant only when t2mfm_rc is
     * T2MFM_SQLITE_ERROR */
    char *sqlite_errmsg;

    /* A doubly linked list of dimension meta-data */
    t2mfm_dll_node *dim_meta_head;
    t2mfm_vec dim_meta_vec; /* A vector of dimension meta-data */
    t2mfm_vec dim_data_vec; /* A vector of dimension data */
    sqlite3_stmt *select_stmt, *insert_stmt, *update_stmt;

    t2mfm_dll_node *itr_head; /* A doubly linked list of iterators */

    int state;
} t2mfm;

/* The Trace2Model Feature Matrix Iterator. A feature matrix iterator handle,
 * usually abbreviated as fmitrh is a pointer to this structure. */
typedef struct t2mfm_itr
{
    t2mfm *fmh;
    int rc;
    char *errmsg;

    int sqlite_rc;
    char *sqlite_errmsg;

    t2mfm_dll_node *filter_info_head;
    /* projection_info_vec->pp_data has t2mfm_bool values embedded into it.
     * Do not perform a deep free */
    t2mfm_vec projection_info_vec;
    t2mfm_dll_node *ordering_info_head;

    t2mfm_vec meta_vec;
    t2mfm_vec data_vec;

    sqlite3_stmt *select_stmt;
} t2mfm_itr;

/* Codes returned by functions that use the feature matrix API. */
typedef enum t2mfm_rc
{
    T2MFM_OK,
    T2MFM_GENERIC_ERROR,
    T2MFM_SQLITE_ERROR,
    T2MFM_EINVAL,
    T2MFM_EINVAL_MODE,
    T2MFM_EINVAL_STATE,
    T2MFM_ENOMEM,
    T2MFM_NEW_ROW,
    T2MFM_NO_MORE_ROWS,
    T2MFM_MATRIX_MISSING,
    T2MFM_MATRIX_EXISTS,
    T2MFM_NOT_IMPLEMENTED,
    T2MFM_EINVAL_METADATA,
    T2MFM_EINVAL_PROJECTION,

    T2MFM_RC_MAX
} t2mfm_rc;

/* Dimension types. */
typedef enum t2mfm_dim_type
{
    T2MFM_INT1, /* One byte signed integer */
    T2MFM_INT4, /* Four byte signed integer */
    T2MFM_INT8, /* Eight byte signed integer */
    T2MFM_REAL,  /* An IEEE floating point number with 8 byte precision */

    T2MFM_DIM_TYPE_MAX
} t2mfm_dim_type;

/* Dimension meta-data */
typedef struct t2mfm_dim_meta
{
    int ordinal; /* The position of this dimension, starting from 0 */
    char *name; /* NULL terminated string */
    char *desc; /* Description. NULL terminated string */
    t2mfm_dim_type datatype; /* Data type */
    t2mfm_bool is_indexed; /* Support fast ordered iterations? */
} t2mfm_dim_meta;

/* Dimension data. The size of this union should be kept small, preferably
 * within two machine words or less. Use indirection (i.e add pointer types)
 * for bigger data. */
typedef union t2mfm_dim_data
{
    int8_t  INT1;
    int32_t INT4;
    int64_t INT8;
    double  REAL;
} t2mfm_dim_data;

/* Statistic specifiers */
typedef enum t2mfm_stat_type
{
    T2MFM_STAT_AVG,
    T2MFM_STAT_MAX,
    T2MFM_STAT_MIN,
    T2MFM_STAT_SUM,
    T2MFM_STAT_COUNT,

    T2MFM_STAT_END
} t2mfm_stat_type;

/*******************************************************************************
 * t2mfm_open: Open or create a feature matrix.
 *
 * @backing_store_uri: A Uniform Resource Identifier that points to a feature
 *   matrix backing store. File URI's are of the form 'file:///foo/bar/baz.ext'.
 *
 * @matrix_name: The name of the feature matrix within @backing_store_uri. The
 *   matrix must not exist in the T2MFM_OPEN_CREATE and must exist in the
 *   T2MFM_OPEN_ITERATE mode.
 *
 * @open_mode: The mode in which the feature matrix must be opened. The
 *   The backing store must exist when when the matrix is opened in the
 *   T2MFM_OPEN_ITERATE mode. A new backing store is created in case it does not
 *   exist when a feature matrix is opened in the T2MFM_OPEN_CREATE mode.
 *
 * @p_fmh: A pointer to a t2mfm handle that has to be initialized. @p_fmh is
 *   initialized even when we fail to create or open the underlying store. A
 *   NULL handle is returned only in rare cases, like when we run out of memory.
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_open(const char *backing_store_uri, const char *matrix_name,
		t2mfm_open_mode open_mode, t2mfm **p_fmh);

/******************************************************************************
 * t2mfm_add_dim_meta_begin: Prepare the matrix to accept new dimension
 *   meta-data.
 *
 * @fmh: A handle to a valid feature matrix opened in the T2MFM_OPEN_CREATE
 *   mode.
 *
 * @postcondition: Feature matrix can accept new dimensions. Use
 *   t2mfm_add_dim_meta to add new dimension meta-data.
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_add_dim_meta_begin(t2mfm *fmh);

/******************************************************************************
 * t2mfm_add_dim_meta: Add a dimension to a feature matrix.
 *
 * @precondition: Successful invocation of t2mfm_add_dim_meta_begin.
 *
 * @fmh: Feature matrix handle.
 *
 * @dim: The dimension to be added. The ordinal is automatically assigned in the
 *   order in which the field is added.
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_add_dim_meta(t2mfm *fmh, const t2mfm_dim_meta *p_dim_meta);

/******************************************************************************
 * t2mfm_add_dim_meta_end: Complete adding dimensions.
 *
 * @fmh: A handle to a valid feature matrix opened in the T2MFM_OPEN_CREATE
 *   mode.
 *
 * @postcondition: No new dimensions can be added.
 ******************************************************************************/
void t2mfm_add_dim_meta_end(t2mfm *fmh);

/******************************************************************************
 * t2mfm_get_dim_meta: Fetch all dimensions.
 *
 * @fmh: Feature matrix handle.
 *
 * @p_dim_meta_vec: A pointer to a vector that will be populated with the
 *   result. The type of every element within (p_dim_meta_vec->pp_data[]) will
 *   be (t2mfm_dim_meta *).
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_get_dim_meta(t2mfm *fmh, t2mfm_vec *p_dim_meta_vec);

/******************************************************************************
 * t2mfm_insert_begin: Prepares the feature matrix for inserts.
 *
 * @precondition: Invocation of t2mfm_add_dim_meta_end.
 *
 * @fmh: Feature matrix handle.
 *
 * @p_dim_data_vec: A pointer to a vector that facilitates data transfer between
 *   the matrix and the client during inserts. The type of every element within
 *   (p_dim_data_vec->pp_data[]) is (t2mfm_dim_data *).
 *
 * @postcondition: New points can be inserted into the matrix using
 *   t2mfm_insert.
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_insert_begin(t2mfm *fmh, t2mfm_vec *p_dim_data_vec);

/*******************************************************************************
 * t2mfm_insert: Add a new point to the matrix. Values are picked up from the
 *   p_dim_data_vec returned during the invocation of t2mfm_insert_begin.
 *
 * @precondition: Successful invocation of t2mfm_insert_begin.
 *
 * @fmh: A handle to a valid feature matrix opened in the T2MFM_OPEN_CREATE
 *   mode.
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_insert(t2mfm *fmh);

/*******************************************************************************
 * t2mfm_insert_end: Completes all insert processing and prepares the matrix
 *   for iterations.
 *
 * @fmh: A handle to a valid feature matrix opened in the T2MFM_OPEN_CREATE
 *   mode.
 *
 * @postcondition: The handle is modified such that any subsequent calls behave
 *   as if the matrix was opened in the T2MFM_OPEN_ITERATE mode.
 ******************************************************************************/
void t2mfm_insert_end(t2mfm *fmh);

/*******************************************************************************
 * t2mfm_itr_alloc: Allocate a new iterator.
 *
 * @fmh: A handle to a valid feature matrix opened in the T2MFM_OPEN_ITERATE
 *   mode.
 *
 * @p_fmitrh: Pointer to a feature matrix iterator handle to be populated.
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_itr_alloc(t2mfm *fmh, t2mfm_itr **p_fmitrh);

/*******************************************************************************
 * t2mfm_itr_free: Close current iterator.
 *
 * @fmh: A handle to a valid feature matrix opened in the T2MFM_OPEN_ITERATE
 *   mode.
 ******************************************************************************/
void t2mfm_itr_free(t2mfm_itr *fmitrh);

/*******************************************************************************
 * t2mfm_get_stat: Compute statistics for a given dimension.
 *
 * @fmh: A handle to a valid feature matrix opened in the T2MFM_OPEN_ITERATE
 *   mode.
 *
 * @stat_type: The type of statistic requested.
 *
 * @stat_val: The resulting value.
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_get_stat(t2mfm *fmh, int dim_ordinal, t2mfm_stat_type stat_type,
        t2mfm_dim_data *stat_val);

/******************************************************************************
 * t2mfm_close: Close a frequency matrix. Also frees up all iterators allocated
 *   for this matrix.
 *
 * @fmh: A (possibly NULL) feature matrix handle.
 ******************************************************************************/
void t2mfm_close(t2mfm *fmh);

/*******************************************************************************
 * t2mfm_get_matrix_names: Return all matrices within a backing store.
 *   Administrative method.
 *
 * @backing_store_uri: The URI identifying the backing store.
 *
 * @p_head: Pointer to the head pointer of a doubly linked list.
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_get_matrix_names(const char *backing_store_uri,
        t2mfm_dll_node *p_head);

/*******************************************************************************
 * t2mfm_strerror: Return the current error message description.
 *
 * @fmh: A (possibly NULL) handle to a valid feature matrix.
 *
 * @return: A NULL terminated character string that contains the error message.
 *   Clients should not modify or free the message string.
 ******************************************************************************/
char *t2mfm_strerror(t2mfm *fmh);

/* Useful error handling macro */
#define T2MFM_HANDLE_RC(handle, rc, goto_label)   \
    {\
        if (rc != T2MFM_OK) {\
            fprintf(stderr, "Error %d: %s\n", rc, t2mfm_strerror(handle));\
            goto goto_label;\
        }\
    }

#define T2MFM_SET_DIM_DATA(dim_data_vec, dim_ordinal, t2mfm_type, dim_value)  \
    {\
        assert((dim_ordinal) < (dim_data_vec).n_elements); \
        ((t2mfm_dim_data *)(dim_data_vec).pp_data[(dim_ordinal)])->t2mfm_type \
            = dim_value; \
    }

#define T2MFM_GET_DIM_DATA(dim__data_vec, dim_ordinal, t2mfm_type) \
        ((t2mfm_dim_data *)(dim_data_vec).pp_data[(dim_ordinal)])->t2mfm_type

/* Administrative methods to completely clear out all matrices in a backing
 * store are not included in the API. If such functionality is needed, use
 * SQL on the backing store directly. */

#endif /* FEATURE_MATRIX_H_ */
