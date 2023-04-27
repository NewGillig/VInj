static void setup_buffer_inter(VP9_COMP *cpi, MACROBLOCK *x, const TileInfo *const tile, MV_REFERENCE_FRAME ref_frame, BLOCK_SIZE block_size, int mi_row, int mi_col, int_mv frame_nearest_mv[MAX_REF_FRAMES], int_mv frame_near_mv[MAX_REF_FRAMES], struct buf_2d yv12_mb[4][MAX_MB_PLANE])
{
    const VP9_COMMON *cm = &cpi->common;
    const YV12_BUFFER_CONFIG *yv12 = get_ref_frame_buffer(cpi, ref_frame);
    MACROBLOCKD *const xd = &x->e_mbd;
    MODE_INFO *const mi = xd->mi[0].src_mi;
    int_mv *const candidates = mi->mbmi.ref_mvs[ref_frame];
    const struct scale_factors *const sf = &cm->frame_refs[ref_frame - 1].sf;
    vp9_setup_pred_block(xd, yv12_mb[ref_frame], yv12, mi_row, mi_col, sf, sf);
    vp9_find_mv_refs(cm, xd, tile, mi, ref_frame, candidates, mi_row, mi_col);
    vp9_find_best_ref_mvs(xd, cm->allow_high_precision_mv, candidates, &frame_nearest_mv[ref_frame], &frame_near_mv[ref_frame]);
    if (!vp9_is_scaled(sf) && block_size >= BLOCK_8X8)
        vp9_mv_pred(cpi, x, yv12_mb[ref_frame][0].buf, yv12->y_stride, ref_frame, block_size);
}