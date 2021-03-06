/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * (C) 2014 by Argonne National Laboratory.
 *     See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include "csp.h"

int MPI_Win_flush_local_all(MPI_Win win)
{
    int mpi_errno = MPI_SUCCESS;

    CSP_DBG_PRINT_FCNAME();

    /* TODO: implement real flush_local_all */
    mpi_errno = MPI_Win_flush_all(win);
    if (mpi_errno != MPI_SUCCESS)
        goto fn_fail;

  fn_exit:
    return mpi_errno;

  fn_fail:
    goto fn_exit;
}
