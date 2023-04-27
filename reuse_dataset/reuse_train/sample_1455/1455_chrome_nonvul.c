static void super_block_yrd(VP9_COMP *cpi, MACROBLOCK *x, int *rate, int64_t *distortion, int *skip, int64_t *psse, BLOCK_SIZE bs, int64_t txfm_cache[TX_MODES], int64_t ref_best_rd)
{
    MACROBLOCKD *xd = &x->e_mbd;
    int64_t sse;
    int64_t *ret_sse = psse ? psse : &sse;
    assert(bs == xd->mi[0].src_mi->mbmi.sb_type);
    if (cpi->sf.tx_size_search_method == USE_LARGESTALL || xd->lossless)
    {
        vpx_memset(txfm_cache, 0, TX_MODES * sizeof(int64_t));
        choose_largest_tx_size(cpi, x, rate, distortion, skip, ret_sse, ref_best_rd, bs);
    }
    else
    {
        choose_tx_size_from_rd(cpi, x, rate, distortion, skip, ret_sse, txfm_cache, ref_best_rd, bs);
    }
}