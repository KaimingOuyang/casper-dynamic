#include <stdio.h>
#include <stdlib.h>
#include "mpiasp.h"

static int MPIASP_Get_impl(void *origin_addr, int origin_count,
        MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp,
        int target_count, MPI_Datatype target_datatype, MPI_Win win,
        MPIASP_Win *ua_win) {
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint ua_target_disp = 0;

    ua_target_disp = ua_win->base_asp_offset[target_rank]
            + ua_win->disp_units[target_rank] * target_disp;

    mpi_errno = PMPI_Get(origin_addr, origin_count, origin_datatype,
            ua_win->asp_ranks_in_ua[target_rank], ua_target_disp, target_count,
            target_datatype, win);
    if (mpi_errno != MPI_SUCCESS)
        goto fn_fail;

    MPIASP_DBG_PRINT(
            "MPIASP Get from asp %d instead of target %d, 0x%lx(0x%lx + %d * %ld)\n",
            ua_win->asp_ranks_in_ua[target_rank], target_rank, ua_target_disp,
            ua_win->base_asp_offset[target_rank],
            ua_win->disp_units[target_rank], target_disp);

    fn_exit:
    return mpi_errno;

    fn_fail:
    goto fn_exit;
}

int MPI_Get(void *origin_addr, int origin_count,
        MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp,
        int target_count, MPI_Datatype target_datatype, MPI_Win win) {
    static const char FCNAME[] = "MPI_Get";
    int mpi_errno = MPI_SUCCESS;
    MPIASP_Win *ua_win;

    MPIASP_DBG_PRINT_FCNAME();

    /* Replace displacement if it is an MPIASP-window */
    mpi_errno = get_ua_win(win, &ua_win);
    if (mpi_errno != MPI_SUCCESS)
        goto fn_fail;

    if (ua_win > 0) {
        mpi_errno = MPIASP_Get_impl(origin_addr, origin_count, origin_datatype,
                target_rank, target_disp, target_count, target_datatype, win,
                ua_win);
    } else {
        mpi_errno = PMPI_Get(origin_addr, origin_count, origin_datatype,
                target_rank, target_disp, target_count, target_datatype, win);
    }

    fn_exit:
    return mpi_errno;

    fn_fail:
    goto fn_exit;
}
