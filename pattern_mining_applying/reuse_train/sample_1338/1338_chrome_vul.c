void vp9_mv_pred(VP9_COMP *cpi, MACROBLOCK *x, uint8_t *ref_y_buffer, int ref_y_stride, int ref_frame, BLOCK_SIZE block_size)
{
    MACROBLOCKD *xd = &x->e_mbd;
    MB_MODE_INFO *mbmi = &xd->mi[0]->mbmi;
    int i;
    int zero_seen = 0;
    int best_index = 0;
    int best_sad = INT_MAX;
    int this_sad = INT_MAX;
    int max_mv = 0;
    uint8_t *src_y_ptr = x->plane[0].src.buf;
    uint8_t *ref_y_ptr;
    const int num_mv_refs = MAX_MV_REF_CANDIDATES + (cpi->sf.adaptive_motion_search && block_size < cpi->sf.max_partition_size);
    MV pred_mv[3];
    pred_mv[0] = mbmi->ref_mvs[ref_frame][0].as_mv;
    pred_mv[1] = mbmi->ref_mvs[ref_frame][1].as_mv;
    pred_mv[2] = x->pred_mv[ref_frame];
    for (i = 0; i < num_mv_refs; ++i)
    {
        const MV *this_mv = &pred_mv[i];
        max_mv = MAX(max_mv, MAX(abs(this_mv->row), abs(this_mv->col)) >> 3);
        if (is_zero_mv(this_mv) && zero_seen)
            continue;
        zero_seen |= is_zero_mv(this_mv);
        ref_y_ptr = &ref_y_buffer[ref_y_stride * (this_mv->row >> 3) + (this_mv->col >> 3)];
        this_sad = cpi->fn_ptr[block_size].sdf(src_y_ptr, x->plane[0].src.stride, ref_y_ptr, ref_y_stride);
        if (this_sad < best_sad)
        {
            best_sad = this_sad;
            best_index = i;
        }
    }
    x->mv_best_ref_index[ref_frame] = best_index;
    x->max_mv_context[ref_frame] = max_mv;
    x->pred_mv_sad[ref_frame] = best_sad;
}