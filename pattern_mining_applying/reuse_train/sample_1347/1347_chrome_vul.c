static int combined_motion_search(VP9_COMP *cpi, MACROBLOCK *x, BLOCK_SIZE bsize, int mi_row, int mi_col, int_mv *tmp_mv, int *rate_mv, int64_t best_rd_sofar)
{
    MACROBLOCKD *xd = &x->e_mbd;
    MB_MODE_INFO *mbmi = &xd->mi[0]->mbmi;
    struct buf_2d backup_yv12[MAX_MB_PLANE] = {{0, 0}};
    const int step_param = cpi->sf.mv.fullpel_search_step_param;
    const int sadpb = x->sadperbit16;
    MV mvp_full;
    const int ref = mbmi->ref_frame[0];
    const MV ref_mv = mbmi->ref_mvs[ref][0].as_mv;
    int dis;
    int rate_mode;
    const int tmp_col_min = x->mv_col_min;
    const int tmp_col_max = x->mv_col_max;
    const int tmp_row_min = x->mv_row_min;
    const int tmp_row_max = x->mv_row_max;
    int rv = 0;
    int sad_list[5];
    const YV12_BUFFER_CONFIG *scaled_ref_frame = vp9_get_scaled_ref_frame(cpi, ref);
    if (cpi->common.show_frame && (x->pred_mv_sad[ref] >> 3) > x->pred_mv_sad[LAST_FRAME])
        return rv;
    if (scaled_ref_frame)
    {
        int i;
        for (i = 0; i < MAX_MB_PLANE; i++)
            backup_yv12[i] = xd->plane[i].pre[0];
        vp9_setup_pre_planes(xd, 0, scaled_ref_frame, mi_row, mi_col, NULL);
    }
    vp9_set_mv_search_range(x, &ref_mv);
    assert(x->mv_best_ref_index[ref] <= 2);
    if (x->mv_best_ref_index[ref] < 2)
        mvp_full = mbmi->ref_mvs[ref][x->mv_best_ref_index[ref]].as_mv;
    else
        mvp_full = x->pred_mv[ref];
    mvp_full.col >>= 3;
    mvp_full.row >>= 3;
    vp9_full_pixel_search(cpi, x, bsize, &mvp_full, step_param, sadpb, cond_sad_list(cpi, sad_list), &ref_mv, &tmp_mv->as_mv, INT_MAX, 0);
    x->mv_col_min = tmp_col_min;
    x->mv_col_max = tmp_col_max;
    x->mv_row_min = tmp_row_min;
    x->mv_row_max = tmp_row_max;
    mvp_full.row = tmp_mv->as_mv.row * 8;
    mvp_full.col = tmp_mv->as_mv.col * 8;
    *rate_mv = vp9_mv_bit_cost(&mvp_full, &ref_mv, x->nmvjointcost, x->mvcost, MV_COST_WEIGHT);
    rate_mode = cpi->inter_mode_cost[mbmi->mode_context[ref]][INTER_OFFSET(NEWMV)];
    rv = !(RDCOST(x->rdmult, x->rddiv, (*rate_mv + rate_mode), 0) > best_rd_sofar);
    if (rv)
    {
        cpi->find_fractional_mv_step(x, &tmp_mv->as_mv, &ref_mv, cpi->common.allow_high_precision_mv, x->errorperbit, &cpi->fn_ptr[bsize], cpi->sf.mv.subpel_force_stop, cpi->sf.mv.subpel_iters_per_step, cond_sad_list(cpi, sad_list), x->nmvjointcost, x->mvcost, &dis, &x->pred_sse[ref], NULL, 0, 0);
        x->pred_mv[ref] = tmp_mv->as_mv;
    }
    if (scaled_ref_frame)
    {
        int i;
        for (i = 0; i < MAX_MB_PLANE; i++)
            xd->plane[i].pre[0] = backup_yv12[i];
    }
    return rv;
}