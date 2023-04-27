static void txfm_rd_in_plane(MACROBLOCK *x, int *rate, int64_t *distortion, int *skippable, int64_t *sse, int64_t ref_best_rd, int plane, BLOCK_SIZE bsize, TX_SIZE tx_size, int use_fast_coef_casting)
{
    MACROBLOCKD *const xd = &x->e_mbd;
    const struct macroblockd_plane *const pd = &xd->plane[plane];
    struct rdcost_block_args args;
    vp9_zero(args);
    args.x = x;
    args.best_rd = ref_best_rd;
    args.use_fast_coef_costing = use_fast_coef_casting;
    if (plane == 0)
        xd->mi[0].src_mi->mbmi.tx_size = tx_size;
    vp9_get_entropy_contexts(bsize, tx_size, pd, args.t_above, args.t_left);
    args.so = get_scan(xd, tx_size, pd->plane_type, 0);
    vp9_foreach_transformed_block_in_plane(xd, bsize, plane, block_rd_txfm, &args);
    if (args.skip)
    {
        *rate = INT_MAX;
        *distortion = INT64_MAX;
        *sse = INT64_MAX;
        *skippable = 0;
    }
    else
    {
        *distortion = args.this_dist;
        *rate = args.this_rate;
        *sse = args.this_sse;
        *skippable = vp9_is_skippable_in_plane(x, bsize, plane);
    }
}