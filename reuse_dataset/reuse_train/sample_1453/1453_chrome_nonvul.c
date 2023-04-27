static void joint_motion_search(VP9_COMP *cpi, MACROBLOCK *x, BLOCK_SIZE bsize, int_mv *frame_mv, int mi_row, int mi_col, int_mv single_newmv[MAX_REF_FRAMES], int *rate_mv)
{
    const int pw = 4 * num_4x4_blocks_wide_lookup[bsize];
    const int ph = 4 * num_4x4_blocks_high_lookup[bsize];
    MACROBLOCKD *xd = &x->e_mbd;
    MB_MODE_INFO *mbmi = &xd->mi[0].src_mi->mbmi;
    const int refs[2] = {mbmi->ref_frame[0], mbmi->ref_frame[1] < 0 ? 0 : mbmi->ref_frame[1]};
    int_mv ref_mv[2];
    int ite, ref;
    uint8_t *second_pred = vpx_memalign(16, pw * ph * sizeof(uint8_t));
    const InterpKernel *kernel = vp9_get_interp_kernel(mbmi->interp_filter);
    struct buf_2d backup_yv12[2][MAX_MB_PLANE];
    struct buf_2d scaled_first_yv12 = xd->plane[0].pre[0];
    int last_besterr[2] = {INT_MAX, INT_MAX};
    const YV12_BUFFER_CONFIG *const scaled_ref_frame[2] = {vp9_get_scaled_ref_frame(cpi, mbmi->ref_frame[0]), vp9_get_scaled_ref_frame(cpi, mbmi->ref_frame[1])};
    for (ref = 0; ref < 2; ++ref)
    {
        ref_mv[ref] = mbmi->ref_mvs[refs[ref]][0];
        if (scaled_ref_frame[ref])
        {
            int i;
            for (i = 0; i < MAX_MB_PLANE; i++)
                backup_yv12[ref][i] = xd->plane[i].pre[ref];
            vp9_setup_pre_planes(xd, ref, scaled_ref_frame[ref], mi_row, mi_col, NULL);
        }
        frame_mv[refs[ref]].as_int = single_newmv[refs[ref]].as_int;
    }
    for (ite = 0; ite < 4; ite++)
    {
        struct buf_2d ref_yv12[2];
        int bestsme = INT_MAX;
        int sadpb = x->sadperbit16;
        MV tmp_mv;
        int search_range = 3;
        int tmp_col_min = x->mv_col_min;
        int tmp_col_max = x->mv_col_max;
        int tmp_row_min = x->mv_row_min;
        int tmp_row_max = x->mv_row_max;
        int id = ite % 2;
        ref_yv12[0] = xd->plane[0].pre[0];
        ref_yv12[1] = xd->plane[0].pre[1];
        vp9_build_inter_predictor(ref_yv12[!id].buf, ref_yv12[!id].stride, second_pred, pw, &frame_mv[refs[!id]].as_mv, &xd->block_refs[!id]->sf, pw, ph, 0, kernel, MV_PRECISION_Q3, mi_col * MI_SIZE, mi_row * MI_SIZE);
        if (id)
            xd->plane[0].pre[0] = ref_yv12[id];
        vp9_set_mv_search_range(x, &ref_mv[id].as_mv);
        tmp_mv = frame_mv[refs[id]].as_mv;
        tmp_mv.col >>= 3;
        tmp_mv.row >>= 3;
        bestsme = vp9_refining_search_8p_c(x, &tmp_mv, sadpb, search_range, &cpi->fn_ptr[bsize], &ref_mv[id].as_mv, second_pred);
        if (bestsme < INT_MAX)
            bestsme = vp9_get_mvpred_av_var(x, &tmp_mv, &ref_mv[id].as_mv, second_pred, &cpi->fn_ptr[bsize], 1);
        x->mv_col_min = tmp_col_min;
        x->mv_col_max = tmp_col_max;
        x->mv_row_min = tmp_row_min;
        x->mv_row_max = tmp_row_max;
        if (bestsme < INT_MAX)
        {
            int dis;
            unsigned int sse;
            bestsme = cpi->find_fractional_mv_step(x, &tmp_mv, &ref_mv[id].as_mv, cpi->common.allow_high_precision_mv, x->errorperbit, &cpi->fn_ptr[bsize], 0, cpi->sf.mv.subpel_iters_per_step, NULL, x->nmvjointcost, x->mvcost, &dis, &sse, second_pred, pw, ph);
        }
        if (id)
            xd->plane[0].pre[0] = scaled_first_yv12;
        if (bestsme < last_besterr[id])
        {
            frame_mv[refs[id]].as_mv = tmp_mv;
            last_besterr[id] = bestsme;
        }
        else
        {
            break;
        }
    }
    *rate_mv = 0;
    for (ref = 0; ref < 2; ++ref)
    {
        if (scaled_ref_frame[ref])
        {
            int i;
            for (i = 0; i < MAX_MB_PLANE; i++)
                xd->plane[i].pre[ref] = backup_yv12[ref][i];
        }
        *rate_mv += vp9_mv_bit_cost(&frame_mv[refs[ref]].as_mv, &mbmi->ref_mvs[refs[ref]][0].as_mv, x->nmvjointcost, x->mvcost, MV_COST_WEIGHT);
    }
    vpx_free(second_pred);
}