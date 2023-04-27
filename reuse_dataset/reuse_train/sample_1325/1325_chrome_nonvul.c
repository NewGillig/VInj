static int temporal_filter_find_matching_mb_c(VP9_COMP *cpi, uint8_t *arf_frame_buf, uint8_t *frame_ptr_buf, int stride)
{
    MACROBLOCK *const x = &cpi->mb;
    MACROBLOCKD *const xd = &x->e_mbd;
    const MV_SPEED_FEATURES *const mv_sf = &cpi->sf.mv;
    int step_param;
    int sadpb = x->sadperbit16;
    int bestsme = INT_MAX;
    int distortion;
    unsigned int sse;
    int sad_list[5];
    MV best_ref_mv1 = {0, 0};
    MV best_ref_mv1_full;
    MV *ref_mv = &x->e_mbd.mi[0].src_mi->bmi[0].as_mv[0].as_mv;
    struct buf_2d src = x->plane[0].src;
    struct buf_2d pre = xd->plane[0].pre[0];
    best_ref_mv1_full.col = best_ref_mv1.col >> 3;
    best_ref_mv1_full.row = best_ref_mv1.row >> 3;
    x->plane[0].src.buf = arf_frame_buf;
    x->plane[0].src.stride = stride;
    xd->plane[0].pre[0].buf = frame_ptr_buf;
    xd->plane[0].pre[0].stride = stride;
    step_param = mv_sf->reduce_first_step_size;
    step_param = MIN(step_param, MAX_MVSEARCH_STEPS - 2);
    vp9_hex_search(x, &best_ref_mv1_full, step_param, sadpb, 1, cond_sad_list(cpi, sad_list), &cpi->fn_ptr[BLOCK_16X16], 0, &best_ref_mv1, ref_mv);
    bestsme = cpi->find_fractional_mv_step(x, ref_mv, &best_ref_mv1, cpi->common.allow_high_precision_mv, x->errorperbit, &cpi->fn_ptr[BLOCK_16X16], 0, mv_sf->subpel_iters_per_step, cond_sad_list(cpi, sad_list), NULL, NULL, &distortion, &sse, NULL, 0, 0);
    x->plane[0].src = src;
    xd->plane[0].pre[0] = pre;
    return bestsme;
}