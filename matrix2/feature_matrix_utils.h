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

#ifndef FEATURE_MATRIX_UTILS_H_
#define FEATURE_MATRIX_UTILS_H_

#include "feature_matrix.h"

inline void set_errmsg(t2mfm *fmh, const char *errmsg);

inline t2mfm_dll_node *list_add_tail(t2mfm_dll_node *head, void *data);

inline t2mfm_dll_node *list_free_node(t2mfm_dll_node *head, void *data,
        t2mfm_bool deep);

inline size_t list_size(t2mfm_dll_node *head);

inline void list_free(t2mfm_dll_node *head, t2mfm_bool deep_free);

inline void vec_free(t2mfm_vec *vec, t2mfm_bool deep_free);

void get_dim_names(t2mfm *fmh, char *sqlbuf, const size_t sqlbuf_size,
        int *chars_written);

#endif /* FEATURE_MATRIX_UTILS_H_ */
