/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * (C) 2014 by Argonne National Laboratory.
 *     See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include "cspg.h"

int run_g_main(void)
{
    int mpi_errno = MPI_SUCCESS;
    int err_class = 0, errstr_len = 0;
    char err_string[MPI_MAX_ERROR_STRING];
    CSP_func FUNC;
    int user_local_root, user_nprocs, user_local_nprocs;

    CSPG_DBG_PRINT(" main start\n");

    /* Disable MPI automatic error messages. */
    mpi_errno = PMPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
    if (mpi_errno != MPI_SUCCESS)
        goto fn_fail;

    /*TODO: init in user app or here ? */
    /*    MPI_Init(&argc, &argv); */
    while (1) {
        mpi_errno = CSPG_func_start(&FUNC, &user_local_root, &user_nprocs, &user_local_nprocs);
        if (mpi_errno != MPI_SUCCESS)
            goto fn_fail;

        switch (FUNC) {
        case CSP_FUNC_WIN_ALLOCATE:
            mpi_errno = CSPG_win_allocate(user_local_root, user_nprocs);
            break;

        case CSP_FUNC_WIN_FREE:
            mpi_errno = CSPG_win_free(user_local_root);
            break;

            /* other commands */
        case CSP_FUNC_ABORT:
            PMPI_Abort(MPI_COMM_WORLD, 1);
            goto fn_exit;

            break;

        case CSP_FUNC_FINALIZE:
            CSPG_finalize();
            goto fn_exit;

            break;

        default:
            /* Skip incorrect FUNC. */
            CSPG_DBG_PRINT(" FUNC %d not supported\n", (int) FUNC);
            break;
        }
    }

    CSPG_DBG_PRINT(" main done\n");

  fn_exit:
    return mpi_errno;

  fn_fail:
    PMPI_Error_class(mpi_errno, &err_class);
    PMPI_Error_string(mpi_errno, err_string, &errstr_len);
    CSPG_ERR_PRINT("MPI reports error code %d, error class %d\n%s",
                   mpi_errno, err_class, err_string);
    goto fn_exit;
}
