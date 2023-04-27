static int64_t rd_pick_best_sub8x8_mode(VP9_COMP *cpi, MACROBLOCK *x, const TileInfo *const tile, int_mv *best_ref_mv, int_mv *second_best_ref_mv, int64_t best_rd, int *returntotrate, int *returnyrate, int64_t *returndistortion, int *skippable, int64_t *psse, int mvthresh, int_mv seg_mvs[4][MAX_REF_FRAMES], BEST_SEG_INFO *bsi_buf, int filter_idx, int mi_row, int mi_col)
{
    int i;
    BEST_SEG_INFO *bsi = bsi_buf + filter_idx;
    MACROBLOCKD *xd = &x->e_mbd;
    MODE_INFO *mi = xd->mi[0];
    MB_MODE_INFO *mbmi = &mi->mbmi;
    int mode_idx;
    int k, br = 0, idx, idy;
    int64_t bd = 0, block_sse = 0;
    PREDICTION_MODE this_mode;
    VP9_COMMON *cm = &cpi->common;
    struct macroblock_plane *const p = &x->plane[0];
    struct macroblockd_plane *const pd = &xd->plane[0];
    const int label_count = 4;
    int64_t this_segment_rd = 0;
    int label_mv_thresh;
    int segmentyrate = 0;
    const BLOCK_SIZE bsize = mbmi->sb_type;
    const int num_4x4_blocks_wide = num_4x4_blocks_wide_lookup[bsize];
    const int num_4x4_blocks_high = num_4x4_blocks_high_lookup[bsize];
    ENTROPY_CONTEXT t_above[2], t_left[2];
    int subpelmv = 1, have_ref = 0;
    const int has_second_rf = has_second_ref(mbmi);
    const int inter_mode_mask = cpi->sf.inter_mode_mask[bsize];
    vp9_zero(*bsi);
    bsi->segment_rd = best_rd;
    bsi->ref_mv[0] = best_ref_mv;
    bsi->ref_mv[1] = second_best_ref_mv;
    bsi->mvp.as_int = best_ref_mv->as_int;
    bsi->mvthresh = mvthresh;
    for (i = 0; i < 4; i++)
        bsi->modes[i] = ZEROMV;
    vpx_memcpy(t_above, pd->above_context, sizeof(t_above));
    vpx_memcpy(t_left, pd->left_context, sizeof(t_left));
    label_mv_thresh = 1 * bsi->mvthresh / label_count;
    for (idy = 0; idy < 2; idy += num_4x4_blocks_high)
    {
        for (idx = 0; idx < 2; idx += num_4x4_blocks_wide)
        {
            int_mv mode_mv[MB_MODE_COUNT][2];
            int_mv frame_mv[MB_MODE_COUNT][MAX_REF_FRAMES];
            PREDICTION_MODE mode_selected = ZEROMV;
            int64_t best_rd = INT64_MAX;
            const int i = idy * 2 + idx;
            int ref;
            for (ref = 0; ref < 1 + has_second_rf; ++ref)
            {
                const MV_REFERENCE_FRAME frame = mbmi->ref_frame[ref];
                frame_mv[ZEROMV][frame].as_int = 0;
                vp9_append_sub8x8_mvs_for_idx(cm, xd, tile, i, ref, mi_row, mi_col, &frame_mv[NEARESTMV][frame], &frame_mv[NEARMV][frame]);
            }
            for (this_mode = NEARESTMV; this_mode <= NEWMV; ++this_mode)
            {
                const struct buf_2d orig_src = x->plane[0].src;
                struct buf_2d orig_pre[2];
                mode_idx = INTER_OFFSET(this_mode);
                bsi->rdstat[i][mode_idx].brdcost = INT64_MAX;
                if (!(inter_mode_mask & (1 << this_mode)))
                    continue;
                if (!check_best_zero_mv(cpi, mbmi->mode_context, frame_mv, inter_mode_mask, this_mode, mbmi->ref_frame))
                    continue;
                vpx_memcpy(orig_pre, pd->pre, sizeof(orig_pre));
                vpx_memcpy(bsi->rdstat[i][mode_idx].ta, t_above, sizeof(bsi->rdstat[i][mode_idx].ta));
                vpx_memcpy(bsi->rdstat[i][mode_idx].tl, t_left, sizeof(bsi->rdstat[i][mode_idx].tl));
                if (!has_second_rf && this_mode == NEWMV && seg_mvs[i][mbmi->ref_frame[0]].as_int == INVALID_MV)
                {
                    MV *const new_mv = &mode_mv[NEWMV][0].as_mv;
                    int step_param = 0;
                    int thissme, bestsme = INT_MAX;
                    int sadpb = x->sadperbit4;
                    MV mvp_full;
                    int max_mv;
                    int sad_list[5];
                    if (best_rd < label_mv_thresh)
                        break;
                    if (cpi->oxcf.mode != BEST)
                    {
                        if (i > 0)
                        {
                            bsi->mvp.as_int = mi->bmi[i - 1].as_mv[0].as_int;
                            if (i == 2)
                                bsi->mvp.as_int = mi->bmi[i - 2].as_mv[0].as_int;
                        }
                    }
                    if (i == 0)
                        max_mv = x->max_mv_context[mbmi->ref_frame[0]];
                    else
                        max_mv = MAX(abs(bsi->mvp.as_mv.row), abs(bsi->mvp.as_mv.col)) >> 3;
                    if (cpi->sf.mv.auto_mv_step_size && cm->show_frame)
                    {
                        step_param = (vp9_init_search_range(max_mv) + cpi->mv_step_param) / 2;
                    }
                    else
                    {
                        step_param = cpi->mv_step_param;
                    }
                    mvp_full.row = bsi->mvp.as_mv.row >> 3;
                    mvp_full.col = bsi->mvp.as_mv.col >> 3;
                    if (cpi->sf.adaptive_motion_search)
                    {
                        mvp_full.row = x->pred_mv[mbmi->ref_frame[0]].row >> 3;
                        mvp_full.col = x->pred_mv[mbmi->ref_frame[0]].col >> 3;
                        step_param = MAX(step_param, 8);
                    }
                    mi_buf_shift(x, i);
                    vp9_set_mv_search_range(x, &bsi->ref_mv[0]->as_mv);
                    bestsme = vp9_full_pixel_search(cpi, x, bsize, &mvp_full, step_param, sadpb, cpi->sf.mv.subpel_search_method != SUBPEL_TREE ? sad_list : NULL, &bsi->ref_mv[0]->as_mv, new_mv, INT_MAX, 1);
                    if (cpi->oxcf.mode == BEST)
                    {
                        int_mv *const best_mv = &mi->bmi[i].as_mv[0];
                        clamp_mv(&mvp_full, x->mv_col_min, x->mv_col_max, x->mv_row_min, x->mv_row_max);
                        thissme = cpi->full_search_sad(x, &mvp_full, sadpb, 16, &cpi->fn_ptr[bsize], &bsi->ref_mv[0]->as_mv, &best_mv->as_mv);
                        sad_list[1] = sad_list[2] = sad_list[3] = sad_list[4] = INT_MAX;
                        if (thissme < bestsme)
                        {
                            bestsme = thissme;
                            *new_mv = best_mv->as_mv;
                        }
                        else
                        {
                            best_mv->as_mv = *new_mv;
                        }
                    }
                    if (bestsme < INT_MAX)
                    {
                        int distortion;
                        cpi->find_fractional_mv_step(x, new_mv, &bsi->ref_mv[0]->as_mv, cm->allow_high_precision_mv, x->errorperbit, &cpi->fn_ptr[bsize], cpi->sf.mv.subpel_force_stop, cpi->sf.mv.subpel_iters_per_step, cond_sad_list(cpi, sad_list), x->nmvjointcost, x->mvcost, &distortion, &x->pred_sse[mbmi->ref_frame[0]], NULL, 0, 0);
                        seg_mvs[i][mbmi->ref_frame[0]].as_mv = *new_mv;
                    }
                    if (cpi->sf.adaptive_motion_search)
                        x->pred_mv[mbmi->ref_frame[0]] = *new_mv;
                    mi_buf_restore(x, orig_src, orig_pre);
                }
                if (has_second_rf)
                {
                    if (seg_mvs[i][mbmi->ref_frame[1]].as_int == INVALID_MV || seg_mvs[i][mbmi->ref_frame[0]].as_int == INVALID_MV)
                        continue;
                }
                if (has_second_rf && this_mode == NEWMV && mbmi->interp_filter == EIGHTTAP)
                {
                    mi_buf_shift(x, i);
                    if (cpi->sf.comp_inter_joint_search_thresh <= bsize)
                    {
                        int rate_mv;
                        joint_motion_search(cpi, x, bsize, frame_mv[this_mode], mi_row, mi_col, seg_mvs[i], &rate_mv);
                        seg_mvs[i][mbmi->ref_frame[0]].as_int = frame_mv[this_mode][mbmi->ref_frame[0]].as_int;
                        seg_mvs[i][mbmi->ref_frame[1]].as_int = frame_mv[this_mode][mbmi->ref_frame[1]].as_int;
                    }
                    mi_buf_restore(x, orig_src, orig_pre);
                }
                bsi->rdstat[i][mode_idx].brate = set_and_cost_bmi_mvs(cpi, xd, i, this_mode, mode_mv[this_mode], frame_mv, seg_mvs[i], bsi->ref_mv, x->nmvjointcost, x->mvcost);
                for (ref = 0; ref < 1 + has_second_rf; ++ref)
                {
                    bsi->rdstat[i][mode_idx].mvs[ref].as_int = mode_mv[this_mode][ref].as_int;
                    if (num_4x4_blocks_wide > 1)
                        bsi->rdstat[i + 1][mode_idx].mvs[ref].as_int = mode_mv[this_mode][ref].as_int;
                    if (num_4x4_blocks_high > 1)
                        bsi->rdstat[i + 2][mode_idx].mvs[ref].as_int = mode_mv[this_mode][ref].as_int;
                }
                if (mv_check_bounds(x, &mode_mv[this_mode][0].as_mv) || (has_second_rf && mv_check_bounds(x, &mode_mv[this_mode][1].as_mv)))
                    continue;
                if (filter_idx > 0)
                {
                    BEST_SEG_INFO *ref_bsi = bsi_buf;
                    subpelmv = 0;
                    have_ref = 1;
                    for (ref = 0; ref < 1 + has_second_rf; ++ref)
                    {
                        subpelmv |= mv_has_subpel(&mode_mv[this_mode][ref].as_mv);
                        have_ref &= mode_mv[this_mode][ref].as_int == ref_bsi->rdstat[i][mode_idx].mvs[ref].as_int;
                    }
                    if (filter_idx > 1 && !subpelmv && !have_ref)
                    {
                        ref_bsi = bsi_buf + 1;
                        have_ref = 1;
                        for (ref = 0; ref < 1 + has_second_rf; ++ref)
                            have_ref &= mode_mv[this_mode][ref].as_int == ref_bsi->rdstat[i][mode_idx].mvs[ref].as_int;
                    }
                    if (!subpelmv && have_ref && ref_bsi->rdstat[i][mode_idx].brdcost < INT64_MAX)
                    {
                        vpx_memcpy(&bsi->rdstat[i][mode_idx], &ref_bsi->rdstat[i][mode_idx], sizeof(SEG_RDSTAT));
                        if (num_4x4_blocks_wide > 1)
                            bsi->rdstat[i + 1][mode_idx].eobs = ref_bsi->rdstat[i + 1][mode_idx].eobs;
                        if (num_4x4_blocks_high > 1)
                            bsi->rdstat[i + 2][mode_idx].eobs = ref_bsi->rdstat[i + 2][mode_idx].eobs;
                        if (bsi->rdstat[i][mode_idx].brdcost < best_rd)
                        {
                            mode_selected = this_mode;
                            best_rd = bsi->rdstat[i][mode_idx].brdcost;
                        }
                        continue;
                    }
                }
                bsi->rdstat[i][mode_idx].brdcost = encode_inter_mb_segment(cpi, x, bsi->segment_rd - this_segment_rd, i, &bsi->rdstat[i][mode_idx].byrate, &bsi->rdstat[i][mode_idx].bdist, &bsi->rdstat[i][mode_idx].bsse, bsi->rdstat[i][mode_idx].ta, bsi->rdstat[i][mode_idx].tl, mi_row, mi_col);
                if (bsi->rdstat[i][mode_idx].brdcost < INT64_MAX)
                {
                    bsi->rdstat[i][mode_idx].brdcost += RDCOST(x->rdmult, x->rddiv, bsi->rdstat[i][mode_idx].brate, 0);
                    bsi->rdstat[i][mode_idx].brate += bsi->rdstat[i][mode_idx].byrate;
                    bsi->rdstat[i][mode_idx].eobs = p->eobs[i];
                    if (num_4x4_blocks_wide > 1)
                        bsi->rdstat[i + 1][mode_idx].eobs = p->eobs[i + 1];
                    if (num_4x4_blocks_high > 1)
                        bsi->rdstat[i + 2][mode_idx].eobs = p->eobs[i + 2];
                }
                if (bsi->rdstat[i][mode_idx].brdcost < best_rd)
                {
                    mode_selected = this_mode;
                    best_rd = bsi->rdstat[i][mode_idx].brdcost;
                }
            }
            if (best_rd == INT64_MAX)
            {
                int iy, midx;
                for (iy = i + 1; iy < 4; ++iy)
                    for (midx = 0; midx < INTER_MODES; ++midx)
                        bsi->rdstat[iy][midx].brdcost = INT64_MAX;
                bsi->segment_rd = INT64_MAX;
                return INT64_MAX;
                ;
            }
            mode_idx = INTER_OFFSET(mode_selected);
            vpx_memcpy(t_above, bsi->rdstat[i][mode_idx].ta, sizeof(t_above));
            vpx_memcpy(t_left, bsi->rdstat[i][mode_idx].tl, sizeof(t_left));
            set_and_cost_bmi_mvs(cpi, xd, i, mode_selected, mode_mv[mode_selected], frame_mv, seg_mvs[i], bsi->ref_mv, x->nmvjointcost, x->mvcost);
            br += bsi->rdstat[i][mode_idx].brate;
            bd += bsi->rdstat[i][mode_idx].bdist;
            block_sse += bsi->rdstat[i][mode_idx].bsse;
            segmentyrate += bsi->rdstat[i][mode_idx].byrate;
            this_segment_rd += bsi->rdstat[i][mode_idx].brdcost;
            if (this_segment_rd > bsi->segment_rd)
            {
                int iy, midx;
                for (iy = i + 1; iy < 4; ++iy)
                    for (midx = 0; midx < INTER_MODES; ++midx)
                        bsi->rdstat[iy][midx].brdcost = INT64_MAX;
                bsi->segment_rd = INT64_MAX;
                return INT64_MAX;
                ;
            }
        }
    }
    bsi->r = br;
    bsi->d = bd;
    bsi->segment_yrate = segmentyrate;
    bsi->segment_rd = this_segment_rd;
    bsi->sse = block_sse;
    for (k = 0; k < 4; ++k)
        bsi->modes[k] = mi->bmi[k].as_mode;
    if (bsi->segment_rd > best_rd)
        return INT64_MAX;
    for (i = 0; i < 4; i++)
    {
        mode_idx = INTER_OFFSET(bsi->modes[i]);
        mi->bmi[i].as_mv[0].as_int = bsi->rdstat[i][mode_idx].mvs[0].as_int;
        if (has_second_ref(mbmi))
            mi->bmi[i].as_mv[1].as_int = bsi->rdstat[i][mode_idx].mvs[1].as_int;
        x->plane[0].eobs[i] = bsi->rdstat[i][mode_idx].eobs;
        mi->bmi[i].as_mode = bsi->modes[i];
    }
    *returntotrate = bsi->r;
    *returndistortion = bsi->d;
    *returnyrate = bsi->segment_yrate;
    *skippable = vp9_is_skippable_in_plane(x, BLOCK_8X8, 0);
    *psse = bsi->sse;
    mbmi->mode = bsi->modes[3];
    return bsi->segment_rd;
}