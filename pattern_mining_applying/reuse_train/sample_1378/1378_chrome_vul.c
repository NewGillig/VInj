static void set_offsets(VP9_COMP *cpi, const TileInfo *const tile, int mi_row, int mi_col, BLOCK_SIZE bsize)
{
    MACROBLOCK *const x = &cpi->mb;
    VP9_COMMON *const cm = &cpi->common;
    MACROBLOCKD *const xd = &x->e_mbd;
    MB_MODE_INFO *mbmi;
    const int mi_width = num_8x8_blocks_wide_lookup[bsize];
    const int mi_height = num_8x8_blocks_high_lookup[bsize];
    const struct segmentation *const seg = &cm->seg;
    set_skip_context(xd, mi_row, mi_col);
    set_modeinfo_offsets(cm, xd, mi_row, mi_col);
    mbmi = &xd->mi[0]->mbmi;
    vp9_setup_dst_planes(xd->plane, get_frame_new_buffer(cm), mi_row, mi_col);
    x->mv_row_min = -(((mi_row + mi_height) * MI_SIZE) + VP9_INTERP_EXTEND);
    x->mv_col_min = -(((mi_col + mi_width) * MI_SIZE) + VP9_INTERP_EXTEND);
    x->mv_row_max = (cm->mi_rows - mi_row) * MI_SIZE + VP9_INTERP_EXTEND;
    x->mv_col_max = (cm->mi_cols - mi_col) * MI_SIZE + VP9_INTERP_EXTEND;
    assert(!(mi_col & (mi_width - 1)) && !(mi_row & (mi_height - 1)));
    set_mi_row_col(xd, tile, mi_row, mi_height, mi_col, mi_width, cm->mi_rows, cm->mi_cols);
    vp9_setup_src_planes(x, cpi->Source, mi_row, mi_col);
    x->rddiv = cpi->rd.RDDIV;
    x->rdmult = cpi->rd.RDMULT;
    if (seg->enabled)
    {
        if (cpi->oxcf.aq_mode != VARIANCE_AQ)
        {
            const uint8_t *const map = seg->update_map ? cpi->segmentation_map : cm->last_frame_seg_map;
            mbmi->segment_id = vp9_get_segment_id(cm, map, bsize, mi_row, mi_col);
        }
        vp9_init_plane_quantizers(cpi, x);
        x->encode_breakout = cpi->segment_encode_breakout[mbmi->segment_id];
    }
    else
    {
        mbmi->segment_id = 0;
        x->encode_breakout = cpi->encode_breakout;
    }
}