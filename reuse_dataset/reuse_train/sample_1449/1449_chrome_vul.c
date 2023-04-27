static void model_rd_for_sb(VP9_COMP *cpi, BLOCK_SIZE bsize, MACROBLOCK *x, MACROBLOCKD *xd, int *out_rate_sum, int64_t *out_dist_sum)
{
    int i;
    int64_t rate_sum = 0;
    int64_t dist_sum = 0;
    const int ref = xd->mi[0]->mbmi.ref_frame[0];
    unsigned int sse;
    unsigned int var = 0;
    unsigned int sum_sse = 0;
    const int shift = 8;
    int rate;
    int64_t dist;
    x->pred_sse[ref] = 0;
    for (i = 0; i < MAX_MB_PLANE; ++i)
    {
        struct macroblock_plane *const p = &x->plane[i];
        struct macroblockd_plane *const pd = &xd->plane[i];
        const BLOCK_SIZE bs = get_plane_block_size(bsize, pd);
        const TX_SIZE max_tx_size = max_txsize_lookup[bs];
        const BLOCK_SIZE unit_size = txsize_to_bsize[max_tx_size];
        int bw = 1 << (b_width_log2_lookup[bs] - b_width_log2_lookup[unit_size]);
        int bh = 1 << (b_height_log2_lookup[bs] - b_width_log2_lookup[unit_size]);
        int idx, idy;
        int lw = b_width_log2_lookup[unit_size] + 2;
        int lh = b_height_log2_lookup[unit_size] + 2;
        sum_sse = 0;
        for (idy = 0; idy < bh; ++idy)
        {
            for (idx = 0; idx < bw; ++idx)
            {
                uint8_t *src = p->src.buf + (idy * p->src.stride << lh) + (idx << lw);
                uint8_t *dst = pd->dst.buf + (idy * pd->dst.stride << lh) + (idx << lh);
                int block_idx = (idy << 1) + idx;
                var = cpi->fn_ptr[unit_size].vf(src, p->src.stride, dst, pd->dst.stride, &sse);
                x->bsse[(i << 2) + block_idx] = sse;
                sum_sse += sse;
                if (!x->select_tx_size)
                {
                    if (x->bsse[(i << 2) + block_idx] < p->quant_thred[0] >> shift)
                        x->skip_txfm[(i << 2) + block_idx] = 1;
                    else if (var<p->quant_thred[1]> > shift)
                        x->skip_txfm[(i << 2) + block_idx] = 2;
                    else
                        x->skip_txfm[(i << 2) + block_idx] = 0;
                }
                if (i == 0)
                    x->pred_sse[ref] += sse;
            }
        }
        if (cpi->oxcf.speed > 4)
        {
            int64_t rate;
            int64_t dist;
            int64_t square_error = sse;
            int quantizer = (pd->dequant[1] >> 3);
            if (quantizer < 120)
                rate = (square_error * (280 - quantizer)) >> 8;
            else
                rate = 0;
            dist = (square_error * quantizer) >> 8;
            rate_sum += rate;
            dist_sum += dist;
        }
        else
        {
            vp9_model_rd_from_var_lapndz(sum_sse, 1 << num_pels_log2_lookup[bs], pd->dequant[1] >> 3, &rate, &dist);
            rate_sum += rate;
            dist_sum += dist;
        }
    }
    *out_rate_sum = (int)rate_sum;
    *out_dist_sum = dist_sum << 4;
}