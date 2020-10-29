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

#include "feature_matrix_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

inline void set_errmsg(t2mfm *fmh, const char *errmsg)
{
	if (!fmh)
		return;

	if (fmh->errmsg)
		free(fmh->errmsg);
	fmh->errmsg = strdup(errmsg);

	if (fmh->db) {
		if (fmh->sqlite_errmsg)
			free(fmh->sqlite_errmsg);
		fmh->sqlite_errmsg = strdup(sqlite3_errmsg(fmh->db));
	}
}

inline t2mfm_dll_node *list_add_tail(t2mfm_dll_node *head, void *data)
{
	t2mfm_dll_node *cur;

	assert(data && "attempt to insert NULL data");

	/* TODO: Handle out of memory */
	cur = (t2mfm_dll_node *) calloc(1, sizeof(t2mfm_dll_node));
	cur->data = data;

	if (head) {
		t2mfm_dll_node *last = head;
		while (last->next) last = last->next;
		last->next = cur;
		cur->prev = last;
		return head;
	} else
		return cur;
}

inline t2mfm_dll_node *list_free_node(t2mfm_dll_node *head, void *data, t2mfm_bool deep)
{
	t2mfm_dll_node *cur;

	if (!head || !data)
		return NULL;

	cur = head;
	while (cur) {
		if (cur->data == data) {
			if (cur->prev)
				cur->prev->next = cur->next;
			else
				head = cur->next;

			if (cur->next)
				cur->next->prev = cur->prev;
			if (deep == T2MFM_TRUE)
				free(data);
			free(cur);
		}
		cur = cur->next;
	}

	return head;
}

inline size_t list_size(t2mfm_dll_node *head)
{
	size_t size = 1;

	if (!head)
		return 0;

	while ((head = head->next)) size++;

	return size;
}

inline void list_free(t2mfm_dll_node *head, t2mfm_bool deep_free)
{
	while (head) {
		t2mfm_dll_node *next = head->next;
		if (deep_free == T2MFM_TRUE && head->data)
			free(head->data);
		free(head);
		head = next;
	}
}

inline void vec_free(t2mfm_vec *vec, t2mfm_bool deep_free)
{
	if (!vec || !vec->pp_data)
		return;

	if (deep_free == T2MFM_TRUE) {
		int i;
		for (i = 0; i < vec->n_elements; i++)
			if (vec->pp_data[i])
				free(vec->pp_data[i]);
	}

	free(vec->pp_data);
}

void get_dim_names(t2mfm *fmh, char *sqlbuf, const size_t sqlbuf_size,
		int *chars_written)
{
	t2mfm_vec *dim_meta_vec;
	int i, cw = 0;

	if (!fmh || !sqlbuf || sqlbuf_size == 0)
		return;

	dim_meta_vec = &fmh->dim_meta_vec;
	for (i = 0; i < dim_meta_vec->n_elements && cw < sqlbuf_size; i++)
		cw += snprintf(sqlbuf + cw, sqlbuf_size - (size_t) cw,
				i < dim_meta_vec->n_elements - 1 ? "%s, " : "%s",
				((t2mfm_dim_meta *) dim_meta_vec->pp_data[i])->name);

	if (chars_written)
		*chars_written += cw;

	return;
}
