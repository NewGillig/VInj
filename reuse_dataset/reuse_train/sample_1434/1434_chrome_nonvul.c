static void single_motion_search(VP9_COMP *cpi, MACROBLOCK *x, BLOCK_SIZE bsize, int mi_row, int mi_col, int_mv *tmp_mv, int *rate_mv)
{
    MACROBLOCKD *xd = &x->e_mbd;
    const VP9_COMMON *cm = &cpi->common;
    MB_MODE_INFO *mbmi = &xd->mi[0].src_mi->mbmi;
    struct buf_2d backup_yv12[MAX_MB_PLANE] = {{0, 0}};
    int bestsme = INT_MAX;
    int step_param;
    int sadpb = x->sadperbit16;
    MV mvp_full;
    int ref = mbmi->ref_frame[0];
    MV ref_mv = mbmi->ref_mvs[ref][0].as_mv;
    int tmp_col_min = x->mv_col_min;
    int tmp_col_max = x->mv_col_max;
    int tmp_row_min = x->mv_row_min;
    int tmp_row_max = x->mv_row_max;
    int sad_list[5];
    const YV12_BUFFER_CONFIG *scaled_ref_frame = vp9_get_scaled_ref_frame(cpi, ref);
    MV pred_mv[3];
    pred_mv[0] = mbmi->ref_mvs[ref][0].as_mv;
    pred_mv[1] = mbmi->ref_mvs[ref][1].as_mv;
    pred_mv[2] = x->pred_mv[ref];
    if (scaled_ref_frame)
    {
        int i;
        for (i = 0; i < MAX_MB_PLANE; i++)
            backup_yv12[i] = xd->plane[i].pre[0];
        vp9_setup_pre_planes(xd, 0, scaled_ref_frame, mi_row, mi_col, NULL);
    }
    vp9_set_mv_search_range(x, &ref_mv);
    if (cpi->sf.mv.auto_mv_step_size && cm->show_frame)
    {
        step_param = (vp9_init_search_range(x->max_mv_context[ref]) + cpi->mv_step_param) / 2;
    }
    else
    {
        step_param = cpi->mv_step_param;
    }
    if (cpi->sf.adaptive_motion_search && bsize < BLOCK_64X64)
    {
        int boffset = 2 * (b_width_log2(BLOCK_64X64) - MIN(b_height_log2(bsize), b_width_log2(bsize)));
        step_param = MAX(step_param, boffset);
    }
    if (cpi->sf.adaptive_motion_search)
    {
        int bwl = b_width_log2(bsize);
        int bhl = b_height_log2(bsize);
        int i;
        int tlevel = x->pred_mv_sad[ref] >> (bwl + bhl + 4);
        if (tlevel < 5)
            step_param += 2;
        for (i = LAST_FRAME; i <= ALTREF_FRAME && cm->show_frame; ++i)
        {
            if ((x->pred_mv_sad[ref] >> 3) > x->pred_mv_sad[i])
            {
                x->pred_mv[ref].row = 0;
                x->pred_mv[ref].col = 0;
                tmp_mv->as_int = INVALID_MV;
                if (scaled_ref_frame)
                {
                    int i;
                    for (i = 0; i < MAX_MB_PLANE; i++)
                        xd->plane[i].pre[0] = backup_yv12[i];
                }
                return;
            }
        }
    }
    mvp_full = pred_mv[x->mv_best_ref_index[ref]];
    mvp_full.col >>= 3;
    mvp_full.row >>= 3;
    bestsme = vp9_full_pixel_search(cpi, x, bsize, &mvp_full, step_param, sadpb, cond_sad_list(cpi, sad_list), &ref_mv, &tmp_mv->as_mv, INT_MAX, 1);
    x->mv_col_min = tmp_col_min;
    x->mv_col_max = tmp_col_max;
    x->mv_row_min = tmp_row_min;
    x->mv_row_max = tmp_row_max;
    if (bestsme < INT_MAX)
    {
        int dis;
        cpi->find_fractional_mv_step(x, &tmp_mv->as_mv, &ref_mv, cm->allow_high_precision_mv, x->errorperbit, &cpi->fn_ptr[bsize], cpi->sf.mv.subpel_force_stop, cpi->sf.mv.subpel_iters_per_step, cond_sad_list(cpi, sad_list), x->nmvjointcost, x->mvcost, &dis, &x->pred_sse[ref], NULL, 0, 0);
    }
    *rate_mv = vp9_mv_bit_cost(&tmp_mv->as_mv, &ref_mv, x->nmvjointcost, x->mvcost, MV_COST_WEIGHT);
    if (cpi->sf.adaptive_motion_search)
        x->pred_mv[ref] = tmp_mv->as_mv;
    if (scaled_ref_frame)
    {
        int i;
        for (i = 0; i < MAX_MB_PLANE; i++)
            xd->plane[i].pre[0] = backup_yv12[i];
    }
}