/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * (C) 2015 by Argonne National Laboratory.
 *     See COPYRIGHT in top-level directory.
 */
#ifndef CASPER_H_
#define CASPER_H_

#include <mpi.h>

/* Get the number of ghost processes. */
int CSP_ghost_size(int *ng);
int CSP_get_verbose(int *verbose);
int CSP_set_verbose(int verbose);

/* Dump asynchronous configuration to specified file.
 * It is a local call. */
void CSP_win_dump_async_config(MPI_Win win, const char *fname);

#endif /* CASPER_H_ */
