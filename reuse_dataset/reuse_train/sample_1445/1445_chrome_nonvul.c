static void choose_intra_uv_mode(VP9_COMP *cpi, PICK_MODE_CONTEXT *ctx, BLOCK_SIZE bsize, TX_SIZE max_tx_size, int *rate_uv, int *rate_uv_tokenonly, int64_t *dist_uv, int *skip_uv, PREDICTION_MODE *mode_uv)
{
    MACROBLOCK *const x = &cpi->mb;
    if (cpi->sf.use_uv_intra_rd_estimate)
    {
        rd_sbuv_dcpred(cpi, x, rate_uv, rate_uv_tokenonly, dist_uv, skip_uv, bsize < BLOCK_8X8 ? BLOCK_8X8 : bsize);
    }
    else
    {
        rd_pick_intra_sbuv_mode(cpi, x, ctx, rate_uv, rate_uv_tokenonly, dist_uv, skip_uv, bsize < BLOCK_8X8 ? BLOCK_8X8 : bsize, max_tx_size);
    }
    *mode_uv = x->e_mbd.mi[0].src_mi->mbmi.uv_mode;
}