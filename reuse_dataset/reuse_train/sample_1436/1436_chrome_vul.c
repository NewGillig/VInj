int64_t vp9_rd_pick_inter_mode_sb_seg_skip(VP9_COMP *cpi, MACROBLOCK *x, int *returnrate, int64_t *returndistortion, BLOCK_SIZE bsize, PICK_MODE_CONTEXT *ctx, int64_t best_rd_so_far)
{
    VP9_COMMON *const cm = &cpi->common;
    RD_OPT *const rd_opt = &cpi->rd;
    MACROBLOCKD *const xd = &x->e_mbd;
    MB_MODE_INFO *const mbmi = &xd->mi[0]->mbmi;
    unsigned char segment_id = mbmi->segment_id;
    const int comp_pred = 0;
    int i;
    int64_t best_tx_diff[TX_MODES];
    int64_t best_pred_diff[REFERENCE_MODES];
    int64_t best_filter_diff[SWITCHABLE_FILTER_CONTEXTS];
    unsigned int ref_costs_single[MAX_REF_FRAMES], ref_costs_comp[MAX_REF_FRAMES];
    vp9_prob comp_mode_p;
    INTERP_FILTER best_filter = SWITCHABLE;
    int64_t this_rd = INT64_MAX;
    int rate2 = 0;
    const int64_t distortion2 = 0;
    x->skip_encode = cpi->sf.skip_encode_frame && x->q_index < QIDX_SKIP_THRESH;
    estimate_ref_frame_costs(cm, xd, segment_id, ref_costs_single, ref_costs_comp, &comp_mode_p);
    for (i = 0; i < MAX_REF_FRAMES; ++i)
        x->pred_sse[i] = INT_MAX;
    for (i = LAST_FRAME; i < MAX_REF_FRAMES; ++i)
        x->pred_mv_sad[i] = INT_MAX;
    *returnrate = INT_MAX;
    assert(vp9_segfeature_active(&cm->seg, segment_id, SEG_LVL_SKIP));
    mbmi->mode = ZEROMV;
    mbmi->uv_mode = DC_PRED;
    mbmi->ref_frame[0] = LAST_FRAME;
    mbmi->ref_frame[1] = NONE;
    mbmi->mv[0].as_int = 0;
    x->skip = 1;
    rd_opt->mask_filter = 0;
    for (i = 0; i < SWITCHABLE_FILTER_CONTEXTS; ++i)
        rd_opt->filter_cache[i] = INT64_MAX;
    if (cm->interp_filter != BILINEAR)
    {
        best_filter = EIGHTTAP;
        if (cm->interp_filter == SWITCHABLE && x->source_variance >= cpi->sf.disable_filter_search_var_thresh)
        {
            int rs;
            int best_rs = INT_MAX;
            for (i = 0; i < SWITCHABLE_FILTERS; ++i)
            {
                mbmi->interp_filter = i;
                rs = vp9_get_switchable_rate(cpi);
                if (rs < best_rs)
                {
                    best_rs = rs;
                    best_filter = mbmi->interp_filter;
                }
            }
        }
    }
    if (cm->interp_filter == SWITCHABLE)
    {
        mbmi->interp_filter = best_filter;
        rate2 += vp9_get_switchable_rate(cpi);
    }
    else
    {
        mbmi->interp_filter = cm->interp_filter;
    }
    if (cm->reference_mode == REFERENCE_MODE_SELECT)
        rate2 += vp9_cost_bit(comp_mode_p, comp_pred);
    rate2 += ref_costs_single[LAST_FRAME];
    this_rd = RDCOST(x->rdmult, x->rddiv, rate2, distortion2);
    *returnrate = rate2;
    *returndistortion = distortion2;
    if (this_rd >= best_rd_so_far)
        return INT64_MAX;
    assert((cm->interp_filter == SWITCHABLE) || (cm->interp_filter == mbmi->interp_filter));
    update_rd_thresh_fact(cpi, bsize, THR_ZEROMV);
    vp9_zero(best_pred_diff);
    vp9_zero(best_filter_diff);
    vp9_zero(best_tx_diff);
    if (!x->select_tx_size)
        swap_block_ptr(x, ctx, 1, 0, 0, MAX_MB_PLANE);
    store_coding_context(x, ctx, THR_ZEROMV, best_pred_diff, best_tx_diff, best_filter_diff, 0);
    return this_rd;
}