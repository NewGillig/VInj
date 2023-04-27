static void update_mbgraph_frame_stats(VP9_COMP *cpi, MBGRAPH_FRAME_STATS *stats, YV12_BUFFER_CONFIG *buf, YV12_BUFFER_CONFIG *golden_ref, YV12_BUFFER_CONFIG *alt_ref)
{
    MACROBLOCK *const x = &cpi->mb;
    MACROBLOCKD *const xd = &x->e_mbd;
    VP9_COMMON *const cm = &cpi->common;
    int mb_col, mb_row, offset = 0;
    int mb_y_offset = 0, arf_y_offset = 0, gld_y_offset = 0;
    MV gld_top_mv = {0, 0};
    MODE_INFO mi_local;
    vp9_zero(mi_local);
    x->mv_row_min = -BORDER_MV_PIXELS_B16;
    x->mv_row_max = (cm->mb_rows - 1) * 8 + BORDER_MV_PIXELS_B16;
    xd->up_available = 0;
    xd->plane[0].dst.stride = buf->y_stride;
    xd->plane[0].pre[0].stride = buf->y_stride;
    xd->plane[1].dst.stride = buf->uv_stride;
    xd->mi[0].src_mi = &mi_local;
    mi_local.mbmi.sb_type = BLOCK_16X16;
    mi_local.mbmi.ref_frame[0] = LAST_FRAME;
    mi_local.mbmi.ref_frame[1] = NONE;
    for (mb_row = 0; mb_row < cm->mb_rows; mb_row++)
    {
        MV gld_left_mv = gld_top_mv;
        int mb_y_in_offset = mb_y_offset;
        int arf_y_in_offset = arf_y_offset;
        int gld_y_in_offset = gld_y_offset;
        x->mv_col_min = -BORDER_MV_PIXELS_B16;
        x->mv_col_max = (cm->mb_cols - 1) * 8 + BORDER_MV_PIXELS_B16;
        xd->left_available = 0;
        for (mb_col = 0; mb_col < cm->mb_cols; mb_col++)
        {
            MBGRAPH_MB_STATS *mb_stats = &stats->mb_stats[offset + mb_col];
            update_mbgraph_mb_stats(cpi, mb_stats, buf, mb_y_in_offset, golden_ref, &gld_left_mv, alt_ref, mb_row, mb_col);
            gld_left_mv = mb_stats->ref[GOLDEN_FRAME].m.mv.as_mv;
            if (mb_col == 0)
            {
                gld_top_mv = gld_left_mv;
            }
            xd->left_available = 1;
            mb_y_in_offset += 16;
            gld_y_in_offset += 16;
            arf_y_in_offset += 16;
            x->mv_col_min -= 16;
            x->mv_col_max -= 16;
        }
        xd->up_available = 1;
        mb_y_offset += buf->y_stride * 16;
        gld_y_offset += golden_ref->y_stride * 16;
        if (alt_ref)
            arf_y_offset += alt_ref->y_stride * 16;
        x->mv_row_min -= 16;
        x->mv_row_max -= 16;
        offset += cm->mb_cols;
    }
}