static void store_coding_context(MACROBLOCK *x, PICK_MODE_CONTEXT *ctx, int mode_index, int64_t comp_pred_diff[REFERENCE_MODES], const int64_t tx_size_diff[TX_MODES], int64_t best_filter_diff[SWITCHABLE_FILTER_CONTEXTS], int skippable)
{
    MACROBLOCKD *const xd = &x->e_mbd;
    ctx->skip = x->skip;
    ctx->skippable = skippable;
    ctx->best_mode_index = mode_index;
    ctx->mic = *xd->mi[0].src_mi;
    ctx->single_pred_diff = (int)comp_pred_diff[SINGLE_REFERENCE];
    ctx->comp_pred_diff = (int)comp_pred_diff[COMPOUND_REFERENCE];
    ctx->hybrid_pred_diff = (int)comp_pred_diff[REFERENCE_MODE_SELECT];
    vpx_memcpy(ctx->tx_rd_diff, tx_size_diff, sizeof(ctx->tx_rd_diff));
    vpx_memcpy(ctx->best_filter_diff, best_filter_diff, sizeof(*best_filter_diff) * SWITCHABLE_FILTER_CONTEXTS);
}