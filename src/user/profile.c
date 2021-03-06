/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * (C) 2014 by Argonne National Laboratory.
 *     See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "csp.h"

#ifdef ENABLE_PROFILE
const char *CSP_profile_func_names[PROF_MAX_NUM_PROFILE_FUNC] = {
    "CSP_MPI_Get",
    "CSP_MPI_Acc",
    "CSP_MPI_Win_flush_all",
    "CSP_MPI_Win_flush_all_gadpt",
    "CSP_MPI_Win_flush_all_main",
    "CSP_MPI_Win_flush",
    "CSP_MPI_Win_flush_gadpt",
    "CSP_MPI_Win_flush_main",
    "CSP_UTIL_coll_async_config"
};

int CSP_prof_counters[PROF_MAX_NUM_PROFILE_FUNC];
CSP_time_t CSP_prof_timings[PROF_MAX_NUM_PROFILE_FUNC];

void CSP_profile_init(void)
{
    int rank, i;
    PMPI_Comm_rank(CSP_COMM_USER_WORLD, &rank);

    for (i = 0; i < PROF_MAX_NUM_PROFILE_FUNC; i++)
        CSP_prof_counters[i] = 0;
    for (i = 0; i < PROF_MAX_NUM_PROFILE_FUNC; i++)
        CSP_prof_timings[i] = 0;

    if (rank == 0) {
        fprintf(stderr, "CSP PROFILE initialized\n");
        fflush(stderr);
    }
}

void CSP_profile_destroy(void)
{
}

void CSP_profile_reset_counter(void)
{
    int i, rank;

    PMPI_Comm_rank(CSP_COMM_USER_WORLD, &rank);
    if (rank == 0) {
        fprintf(stderr, "CSP counter reset\n");
        fflush(stderr);
    }

    for (i = 0; i < PROF_MAX_NUM_PROFILE_FUNC; i++)
        CSP_prof_counters[i] = 0;
}

void CSP_profile_reset_timing(void)
{
    int i, rank;

    PMPI_Comm_rank(CSP_COMM_USER_WORLD, &rank);
    if (rank == 0) {
        fprintf(stderr, "CSP timing reset\n");
        fflush(stderr);
    }

    for (i = 0; i < PROF_MAX_NUM_PROFILE_FUNC; i++)
        CSP_prof_timings[i] = 0;
}

void CSP_profile_print_timing(char *name)
{
    int i, rank, size;
    double timers_avg[PROF_MAX_NUM_PROFILE_FUNC];
    double timers_double[PROF_MAX_NUM_PROFILE_FUNC];

    PMPI_Comm_rank(CSP_COMM_USER_WORLD, &rank);
    PMPI_Comm_size(CSP_COMM_USER_WORLD, &size);

    for (i = 0; i < PROF_MAX_NUM_PROFILE_FUNC; i++)
        timers_double[i] = CSP_time_todouble(CSP_prof_timings[i]);

    PMPI_Reduce(timers_double, timers_avg, PROF_MAX_NUM_PROFILE_FUNC, MPI_DOUBLE, MPI_SUM, 0,
                CSP_COMM_USER_WORLD);

    if (rank == 0) {
        for (i = 0; i < PROF_MAX_NUM_PROFILE_FUNC; i++) {
            if (timers_avg[i] > 0.0) {
                timers_avg[i] = timers_avg[i] / size;
                fprintf(stderr, "%s timers = %s : %lf\n", name, CSP_profile_func_names[i],
                        timers_avg[i]);
            }
        }
        fflush(stderr);
    }
}

void CSP_profile_print_counter(char *name)
{
    int i, dst, rank;
    int counters_avg[PROF_MAX_NUM_PROFILE_FUNC];

    MPI_Comm_rank(CSP_COMM_USER_WORLD, &rank);
    memset(counters_avg, 0, sizeof(int) * PROF_MAX_NUM_PROFILE_FUNC);

    PMPI_Reduce(CSP_prof_counters, counters_avg, PROF_MAX_NUM_PROFILE_FUNC, MPI_INT, MPI_SUM, 0,
                CSP_COMM_USER_WORLD);

    if (rank == 0) {
        for (i = 0; i < PROF_MAX_NUM_PROFILE_FUNC; i++) {
            if (counters_avg[i] > 0) {
                fprintf(stderr, "%s counters = %s : %d\n", name, CSP_profile_func_names[i],
                        counters_avg[i]);
                fflush(stderr);
            }
        }
    }
    fflush(stdout);
}

#endif
