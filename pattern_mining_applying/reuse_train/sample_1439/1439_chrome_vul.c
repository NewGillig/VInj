static void super_block_uvrd(const VP9_COMP *cpi, MACROBLOCK *x, int *rate, int64_t *distortion, int *skippable, int64_t *sse, BLOCK_SIZE bsize, int64_t ref_best_rd)
{
    MACROBLOCKD *const xd = &x->e_mbd;
    MB_MODE_INFO *const mbmi = &xd->mi[0]->mbmi;
    const TX_SIZE uv_tx_size = get_uv_tx_size(mbmi, &xd->plane[1]);
    int plane;
    int pnrate = 0, pnskip = 1;
    int64_t pndist = 0, pnsse = 0;
    if (ref_best_rd < 0)
        goto term;
    if (is_inter_block(mbmi))
    {
        int plane;
        for (plane = 1; plane < MAX_MB_PLANE; ++plane)
            vp9_subtract_plane(x, bsize, plane);
    }
    *rate = 0;
    *distortion = 0;
    *sse = 0;
    *skippable = 1;
    for (plane = 1; plane < MAX_MB_PLANE; ++plane)
    {
        txfm_rd_in_plane(x, &pnrate, &pndist, &pnskip, &pnsse, ref_best_rd, plane, bsize, uv_tx_size, cpi->sf.use_fast_coef_costing);
        if (pnrate == INT_MAX)
            goto term;
        *rate += pnrate;
        *distortion += pndist;
        *sse += pnsse;
        *skippable &= pnskip;
    }
    return;
term:
    *rate = INT_MAX;
    *distortion = INT64_MAX;
    *sse = INT64_MAX;
    *skippable = 0;
    return;
}