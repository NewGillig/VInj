static void set_block_thresholds(const VP9_COMMON *cm, RD_OPT *rd)
{
    int i, bsize, segment_id;
    for (segment_id = 0; segment_id < MAX_SEGMENTS; ++segment_id)
    {
        const int qindex = clamp(vp9_get_qindex(&cm->seg, segment_id, cm->base_qindex) + cm->y_dc_delta_q, 0, MAXQ);
        const int q = compute_rd_thresh_factor(qindex);
        for (bsize = 0; bsize < BLOCK_SIZES; ++bsize)
        {
            const int t = q * rd_thresh_block_size_factor[bsize];
            const int thresh_max = INT_MAX / t;
            if (bsize >= BLOCK_8X8)
            {
                for (i = 0; i < MAX_MODES; ++i)
                    rd->threshes[segment_id][bsize][i] = rd->thresh_mult[i] < thresh_max ? rd->thresh_mult[i] * t / 4 : INT_MAX;
            }
            else
            {
                for (i = 0; i < MAX_REFS; ++i)
                    rd->threshes[segment_id][bsize][i] = rd->thresh_mult_sub8x8[i] < thresh_max ? rd->thresh_mult_sub8x8[i] * t / 4 : INT_MAX;
            }
        }
    }
}