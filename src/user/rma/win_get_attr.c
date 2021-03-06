/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * (C) 2015 by Argonne National Laboratory.
 *     See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include "csp.h"

int MPI_Win_get_attr(MPI_Win win, int win_keyval, void *attribute_val, int *flag)
{
    CSP_win *ug_win;
    int mpi_errno = MPI_SUCCESS;

    CSP_fetch_ug_win_from_cache(win, ug_win);

    if (ug_win == NULL) {
        /* normal window */
        return PMPI_Win_get_attr(win, win_keyval, attribute_val, flag);
    }

    /* Override win_create flavor for casper-allocated window.
     * Only win_create flavor is changed within Casper because the window is
     * internally created by WIN_CREATE. For all the other attributes, just return
     * its real value.*/
    if (win_keyval == MPI_WIN_CREATE_FLAVOR) {
        *((MPIR_Win_flavor_t **) attribute_val) = &ug_win->create_flavor;
        *flag = 1;
    }
    else {
        mpi_errno = PMPI_Win_get_attr(win, win_keyval, attribute_val, flag);
    }

    return mpi_errno;
}
