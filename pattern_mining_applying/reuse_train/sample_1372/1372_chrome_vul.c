static void nonrd_pick_sb_modes(VP9_COMP *cpi, const TileInfo *const tile, int mi_row, int mi_col, int *rate, int64_t *dist, BLOCK_SIZE bsize, PICK_MODE_CONTEXT *ctx)
{
    VP9_COMMON *const cm = &cpi->common;
    MACROBLOCK *const x = &cpi->mb;
    MACROBLOCKD *const xd = &x->e_mbd;
    MB_MODE_INFO *mbmi;
    set_offsets(cpi, tile, mi_row, mi_col, bsize);
    mbmi = &xd->mi[0]->mbmi;
    mbmi->sb_type = bsize;
    if (cpi->oxcf.aq_mode == CYCLIC_REFRESH_AQ && cm->seg.enabled)
        if (mbmi->segment_id && x->in_static_area)
            x->rdmult = vp9_cyclic_refresh_get_rdmult(cpi->cyclic_refresh);
    if (vp9_segfeature_active(&cm->seg, mbmi->segment_id, SEG_LVL_SKIP))
        set_mode_info_seg_skip(x, cm->tx_mode, rate, dist, bsize);
    else
        vp9_pick_inter_mode(cpi, x, tile, mi_row, mi_col, rate, dist, bsize, ctx);
    duplicate_mode_info_in_sb(cm, xd, mi_row, mi_col, bsize);
}