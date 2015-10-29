/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * (C) 2014 by Argonne National Laboratory.
 *     See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include "cspg.h"

/**
 * Ghosts receive a new function from user root process
 */
int CSPG_func_start(CSP_func * FUNC, int *user_local_root, int *user_nprocs, int *user_local_nprocs)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_Status status;
    CSPG_func_info g_info;
    int local_gp_rank = 0;

    PMPI_Comm_rank(CSP_COMM_GHOST_LOCAL, &local_gp_rank);
    memset(&g_info, 0, sizeof(g_info));

    /* Only root ghost receives start request from user roots.
     * Otherwise deadlock may happen if multiple user roots send request to
     * ghosts concurrently and some ghosts are locked in different communicator creation. */
    if (local_gp_rank == 0) {
        MPI_Request recv_req = MPI_REQUEST_NULL;
        int flag = 0;

        mpi_errno = PMPI_Irecv((char *) &g_info, sizeof(CSP_func_info), MPI_CHAR,
                               MPI_ANY_SOURCE, CSP_FUNC_TAG, CSP_COMM_LOCAL, &recv_req);
        if (mpi_errno != MPI_SUCCESS)
            return mpi_errno;

        /* blocking wait for receive */
        while (recv_req != MPI_REQUEST_NULL) {
            mpi_errno = PMPI_Test(&recv_req, &flag, &status);
            if (mpi_errno != MPI_SUCCESS)
                return mpi_errno;

            /* progress on global asynchronous status synchronization. */
            mpi_errno = CSPG_ra_gsync();
            if (mpi_errno != MPI_SUCCESS)
                return mpi_errno;
        }

        CSPG_DBG_PRINT(" received Func start request from local rank %d\n", status.MPI_SOURCE);

        g_info.user_root_in_local = status.MPI_SOURCE;
    }

    /* All other ghosts start from here */
    mpi_errno = PMPI_Bcast((char *) &g_info, sizeof(CSPG_func_info), MPI_CHAR, 0,
                           CSP_COMM_GHOST_LOCAL);
    if (mpi_errno != MPI_SUCCESS)
        return mpi_errno;

    *FUNC = g_info.info.FUNC;
    *user_nprocs = g_info.info.user_nprocs;
    *user_local_nprocs = g_info.info.user_local_nprocs;
    *user_local_root = g_info.user_root_in_local;

    CSPG_DBG_PRINT(" all ghosts started for Func %d, user nprocs %d, local_nprocs %d,"
                   "user_local_root %d\n", (int) (*FUNC), *user_nprocs, *user_local_nprocs,
                   *user_local_root);

    return mpi_errno;
}

int CSPG_func_new_ur_g_comm(int user_local_root, MPI_Comm * ur_g_comm)
{
    int mpi_errno = MPI_SUCCESS;
    int *ur_g_ranks_in_local = NULL;
    MPI_Group ur_g_group = MPI_GROUP_NULL;
    int i;

    /* Create a user root + all ghosts communicator for later information exchanges. */
    ur_g_ranks_in_local = CSP_calloc(sizeof(int), CSP_ENV.num_g + 1);
    /* Ghosts' rank are always start from 0 */
    for (i = 0; i < CSP_ENV.num_g; i++)
        ur_g_ranks_in_local[i] = i;
    ur_g_ranks_in_local[CSP_ENV.num_g] = user_local_root;

    PMPI_Group_incl(CSP_GROUP_LOCAL, CSP_ENV.num_g + 1, ur_g_ranks_in_local, &ur_g_group);
    mpi_errno = PMPI_Comm_create_group(CSP_COMM_LOCAL, ur_g_group, 0, ur_g_comm);
    if (mpi_errno != MPI_SUCCESS)
        goto fn_fail;

#ifdef CSP_DEBUG
    int ur_g_rank, ur_g_nprocs;
    PMPI_Comm_rank(*ur_g_comm, &ur_g_rank);
    PMPI_Comm_size(*ur_g_comm, &ur_g_nprocs);
    CSPG_DBG_PRINT(" created ur_h comm, my rank %d/%d \n", ur_g_rank, ur_g_nprocs);
#endif

  fn_exit:
    if (ur_g_ranks_in_local)
        free(ur_g_ranks_in_local);
    if (ur_g_group != MPI_GROUP_NULL)
        PMPI_Group_free(&ur_g_group);

    return mpi_errno;

  fn_fail:
    goto fn_exit;
}

int CSPG_func_get_param(char *func_params, int size, MPI_Comm ur_g_comm)
{
    /* User root is always the last rank in user root + ghosts communicator */
    CSPG_DBG_PRINT("get param from user root: size %d\n", size);
    return PMPI_Bcast(func_params, size, MPI_CHAR, CSP_ENV.num_g, ur_g_comm);
}
