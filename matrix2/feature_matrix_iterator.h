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

#ifndef FEATURE_MATRIX_ITERATOR_H_
#define FEATURE_MATRIX_ITERATOR_H_

#include "feature_matrix.h"

/* Iteration order specifiers */
typedef enum t2mfm_itr_order
{
    T2MFM_ASC, /* Ascending */
    T2MFM_DESC /* Descending */
} t2mfm_itr_order;

/*******************************************************************************
 * t2m_add_filter: Add a ranger filter to a given dimension. Current support for
 *   range filtering is limited to numeric fields. Multiple range filters on the
 *   same field are currently un-supported.
 *
 * @fmitrh: Feature matrix iterator handle.
 *
 * @dim_ordinal: The dimension to add filter on.
 *
 * @start, @end: The start and end values, both inclusive.
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_add_filter(t2mfm_itr *fmitrh, int dim_ordinal, t2mfm_dim_data start,
        t2mfm_dim_data end);

/*******************************************************************************
 * t2mfm_add_projection: Project a dimension.
 *
  * @fmitrh: Feature matrix iterator handle.
 *
 * @dim_ordinal: The dimension to project.
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_add_projection(t2mfm_itr *fmitrh, int dim_ordinal);

/*******************************************************************************
 * t2mfm_set_itr_order: Set the iteration for a given dimension.
 *
 * @fmitrh: Feature matrix iterator handle.
 *
 * @dim_ordinal: Identifies the dimension.
 *
 * @order: Specifies the order of iteration
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_set_itr_order(t2mfm_itr *fmitrh, int dim_ordinal,
        t2mfm_itr_order order);

/*******************************************************************************
 * t2mfm_itr_begin: Prepare the matrix for point traversal.
 *
 * @fmitrh: Feature matrix iterator handle.
 *
 * @p_dim_data_vec: A pointer to a vector that facilitates data transfer between
 *   the matrix and the client during inserts. The type of every element within
 *   (p_dim_data_vec->pp_data[]) is (t2mfm_dim_data *).
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_itr_begin(t2mfm_itr *fmitrh, t2mfm_vec *p_dim_data_vec);

/******************************************************************************
 * t2mfm_itr_create_new: Create a new feature matrix instead of iterating over
 *   all elements.
 *
 * @fmh: The frequency matrix to  clone.
 *
 * @new_matrix_name: The name of the new feature marix.
 *
 * @p_new_fmh: Pointer to a new frequency matrix handle to be initialized.
 *
 * @return: T2MFM_OK on success, or an appropriate failure code.
 ******************************************************************************/
int t2mfm_itr_create_new_fm(t2mfm_itr *fmitrh, const char *new_matrix_name,
        t2mfm** p_new_fmh);

/*******************************************************************************
 * t2mfm_itr_next: Load the next set of points onto fields.
 *
 * @precondition: Successful invocation of t2mfm_itr_begin.
 *
 * @fmitrh: Feature matrix iterator handle.
 *
 * @return: T2MFM_NEW_ROW if a new row was loaded, T2MFM_NO_MORE_ROWS if there
 *   were no new rows to load, or a different failure code.
 ******************************************************************************/
int t2mfm_itr_next(t2mfm_itr *fmitrh);

/*******************************************************************************
 * t2mfm_strerror: Return the current error message description.
 *
 * @fmitrh: A (possibly NULL) handle to a valid feature matrix iterator.
 *
 * @return: A NULL terminated character string that contains the error message.
 *   Clients should not modify or free the message string.
 ******************************************************************************/
char *t2mfm_itr_strerror(t2mfm_itr *fmitrh);

/* Useful error handling macro */
#define T2MFM_HANDLE_ITR_RC(handle, rc, goto_label)   \
    {\
        if (rc != T2MFM_OK) {\
            fprintf(stderr, "Error %d: %s\n", rc, t2mfm_itr_strerror(handle));\
            goto goto_label;\
        }\
    }

#endif /* FEATURE_MATRIX_ITERATOR_H_ */
