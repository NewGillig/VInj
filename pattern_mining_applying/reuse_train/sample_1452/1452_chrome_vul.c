static int set_and_cost_bmi_mvs(VP9_COMP *cpi, MACROBLOCKD *xd, int i, PREDICTION_MODE mode, int_mv this_mv[2], int_mv frame_mv[MB_MODE_COUNT][MAX_REF_FRAMES], int_mv seg_mvs[MAX_REF_FRAMES], int_mv *best_ref_mv[2], const int *mvjcost, int *mvcost[2])
{
    MODE_INFO *const mic = xd->mi[0];
    const MB_MODE_INFO *const mbmi = &mic->mbmi;
    int thismvcost = 0;
    int idx, idy;
    const int num_4x4_blocks_wide = num_4x4_blocks_wide_lookup[mbmi->sb_type];
    const int num_4x4_blocks_high = num_4x4_blocks_high_lookup[mbmi->sb_type];
    const int is_compound = has_second_ref(mbmi);
    switch (mode)
    {
    case NEWMV:
        this_mv[0].as_int = seg_mvs[mbmi->ref_frame[0]].as_int;
        thismvcost += vp9_mv_bit_cost(&this_mv[0].as_mv, &best_ref_mv[0]->as_mv, mvjcost, mvcost, MV_COST_WEIGHT_SUB);
        if (is_compound)
        {
            this_mv[1].as_int = seg_mvs[mbmi->ref_frame[1]].as_int;
            thismvcost += vp9_mv_bit_cost(&this_mv[1].as_mv, &best_ref_mv[1]->as_mv, mvjcost, mvcost, MV_COST_WEIGHT_SUB);
        }
        break;
    case NEARMV:
    case NEARESTMV:
        this_mv[0].as_int = frame_mv[mode][mbmi->ref_frame[0]].as_int;
        if (is_compound)
            this_mv[1].as_int = frame_mv[mode][mbmi->ref_frame[1]].as_int;
        break;
    case ZEROMV:
        this_mv[0].as_int = 0;
        if (is_compound)
            this_mv[1].as_int = 0;
        break;
    default:
        break;
    }
    mic->bmi[i].as_mv[0].as_int = this_mv[0].as_int;
    if (is_compound)
        mic->bmi[i].as_mv[1].as_int = this_mv[1].as_int;
    mic->bmi[i].as_mode = mode;
    for (idy = 0; idy < num_4x4_blocks_high; ++idy)
        for (idx = 0; idx < num_4x4_blocks_wide; ++idx)
            vpx_memcpy(&mic->bmi[i + idy * 2 + idx], &mic->bmi[i], sizeof(mic->bmi[i]));
    return cost_mv_ref(cpi, mode, mbmi->mode_context[mbmi->ref_frame[0]]) + thismvcost;
}