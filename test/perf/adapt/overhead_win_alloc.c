/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * (C) 2015 by Argonne National Laboratory.
 *     See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <cperf.h>

/* This benchmark measures the overhead of win_allocate with different
 * adaptation modes.*/

#define SKIP 10
#define ITER 10000
#define SUB_ITER 1000
int rank, nprocs;
double *winbuf[SUB_ITER];
MPI_Win win[SUB_ITER];
int size = 1024;
char async_sched_level_str[128] = { 0 };
char cperf_info_async_config[CPERF_ENVVAL_MAXLEN] = { 0 };

static void run_test(const char *info)
{
    int x, sx;
    double t0, t1, t_alloc, t_free;
    MPI_Info win_info = MPI_INFO_NULL;

    MPI_Info_create(&win_info);
    MPI_Info_set(win_info, (char *) "epoch_type", "fence");

    if (info != NULL)
        MPI_Info_set(win_info, (char *) "async_config", info);

    for (x = 0; x < SKIP; x++) {
        MPI_Win_allocate(sizeof(double) * size, sizeof(double), win_info,
                         MPI_COMM_WORLD, &winbuf[x], &win[x]);
        MPI_Win_free(&win[x]);
    }

    t_alloc = t_free = 0;
    for (x = 0; x < ITER; x += SUB_ITER) {
        t0 = MPI_Wtime();
        for (sx = 0; sx < SUB_ITER; sx++) {
            /* size in byte */
            MPI_Win_allocate(sizeof(double) * size, sizeof(double), win_info,
                             MPI_COMM_WORLD, &winbuf[sx], &win[sx]);
        }
        t1 = MPI_Wtime();
        t_alloc += (t1 - t0) / ITER * 1000 * 1000;      /*us */

        /* release window in every sub_iter loops, otherwise may get error from
         * communicator creation in cray mpi. */
        for (sx = 0; sx < SUB_ITER; sx++) {
            MPI_Win_free(&win[sx]);
        }
        t_free += (MPI_Wtime() - t1) / ITER * 1000 * 1000;      /*us */
    }
    if (rank == 0)
        fprintf(stdout, "%s%s, nproc %d config %s allocate %lf free %lf\n",
#ifdef ENABLE_CSP
                "csp-", async_sched_level_str,
#else
                "original", "",
#endif
                nprocs, (info == NULL) ? "none" : info, t_alloc, t_free);

    if (win_info != MPI_INFO_NULL)
        MPI_Info_free(&win_info);
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc > 1) {
        size = atoi(argv[1]);
    }

    if (size <= 0) {
        fprintf(stderr, "wrong size %d\n", size);
        goto exit;
    }

    CTEST_perf_read_env("CSP_ASYNC_SCHED_LEVEL", "per-win", &async_sched_level_str);
    CTEST_perf_read_env("CPERF_INFO_ASYNC_CONFIG", "on", &cperf_info_async_config);

    MPI_Barrier(MPI_COMM_WORLD);
    run_test(cperf_info_async_config);

  exit:
    MPI_Finalize();

    return 0;
}
