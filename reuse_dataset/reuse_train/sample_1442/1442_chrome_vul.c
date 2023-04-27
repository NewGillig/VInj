static void choose_largest_tx_size(VP9_COMP *cpi, MACROBLOCK *x, int *rate, int64_t *distortion, int *skip, int64_t *sse, int64_t ref_best_rd, BLOCK_SIZE bs)
{
    const TX_SIZE max_tx_size = max_txsize_lookup[bs];
    VP9_COMMON *const cm = &cpi->common;
    const TX_SIZE largest_tx_size = tx_mode_to_biggest_tx_size[cm->tx_mode];
    MACROBLOCKD *const xd = &x->e_mbd;
    MB_MODE_INFO *const mbmi = &xd->mi[0]->mbmi;
    mbmi->tx_size = MIN(max_tx_size, largest_tx_size);
    txfm_rd_in_plane(x, rate, distortion, skip, sse, ref_best_rd, 0, bs, mbmi->tx_size, cpi->sf.use_fast_coef_costing);
}