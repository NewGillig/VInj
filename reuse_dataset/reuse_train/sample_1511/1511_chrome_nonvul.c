static void loop_filter_rows_mt(const YV12_BUFFER_CONFIG *const frame_buffer, VP9_COMMON *const cm, struct macroblockd_plane planes[MAX_MB_PLANE], int start, int stop, int y_only, VP9LfSync *const lf_sync, int num_lf_workers)
{
    const int num_planes = y_only ? 1 : MAX_MB_PLANE;
    int r, c;
    const int sb_cols = mi_cols_aligned_to_sb(cm->mi_cols) >> MI_BLOCK_SIZE_LOG2;
    for (r = start; r < stop; r += num_lf_workers)
    {
        const int mi_row = r << MI_BLOCK_SIZE_LOG2;
        MODE_INFO *const mi = cm->mi + mi_row * cm->mi_stride;
        for (c = 0; c < sb_cols; ++c)
        {
            const int mi_col = c << MI_BLOCK_SIZE_LOG2;
            LOOP_FILTER_MASK lfm;
            int plane;
            sync_read(lf_sync, r, c);
            vp9_setup_dst_planes(planes, frame_buffer, mi_row, mi_col);
            vp9_setup_mask(cm, mi_row, mi_col, mi + mi_col, cm->mi_stride, &lfm);
            for (plane = 0; plane < num_planes; ++plane)
            {
                vp9_filter_block_plane(cm, &planes[plane], mi_row, &lfm);
            }
            sync_write(lf_sync, r, c, sb_cols);
        }
    }
}