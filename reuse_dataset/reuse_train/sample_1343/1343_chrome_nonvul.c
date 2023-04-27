void vp9_select_in_frame_q_segment(VP9_COMP *cpi, int mi_row, int mi_col, int output_enabled, int projected_rate)
{
    VP9_COMMON *const cm = &cpi->common;
    const int mi_offset = mi_row * cm->mi_cols + mi_col;
    const int bw = num_8x8_blocks_wide_lookup[BLOCK_64X64];
    const int bh = num_8x8_blocks_high_lookup[BLOCK_64X64];
    const int xmis = MIN(cm->mi_cols - mi_col, bw);
    const int ymis = MIN(cm->mi_rows - mi_row, bh);
    int complexity_metric = 64;
    int x, y;
    unsigned char segment;
    if (!output_enabled)
    {
        segment = 0;
    }
    else
    {
        const int target_rate = (cpi->rc.sb64_target_rate * xmis * ymis * 256) / (bw * bh);
        const int aq_strength = get_aq_c_strength(cm->base_qindex, cm->bit_depth);
        const int active_segments = aq_c_active_segments[aq_strength];
        segment = active_segments - 1;
        while (segment > 0)
        {
            if (projected_rate < (target_rate * aq_c_transitions[aq_strength][segment]))
            {
                break;
            }
            --segment;
        }
        if (target_rate > 0)
        {
            complexity_metric = clamp((int)((projected_rate * 64) / target_rate), 16, 255);
        }
    }
    for (y = 0; y < ymis; y++)
    {
        for (x = 0; x < xmis; x++)
        {
            cpi->segmentation_map[mi_offset + y * cm->mi_cols + x] = segment;
            cpi->complexity_map[mi_offset + y * cm->mi_cols + x] = (unsigned char)complexity_metric;
        }
    }
}