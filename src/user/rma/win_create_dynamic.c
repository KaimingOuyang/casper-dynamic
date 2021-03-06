/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * (C) 2014 by Argonne National Laboratory.
 *     See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include "csp.h"

int MPI_Win_create_dynamic(MPI_Info info, MPI_Comm comm, MPI_Win * win)
{
    int mpi_errno = MPI_SUCCESS;

    CSP_DBG_PRINT_FCNAME();
    CSP_MPI_FUNC_START_ROUTINE();

    if (comm == MPI_COMM_WORLD)
        comm = CSP_COMM_USER_WORLD;
    mpi_errno = PMPI_Win_create_dynamic(info, comm, win);

    CSP_WARN_PRINT("called MPI_Win_create_dynamic, no asynchronous progress on win 0x%x\n", *win);

    CSP_MPI_FUNC_END_ROUTINE();
    return mpi_errno;
}
