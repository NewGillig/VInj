static void rd_pick_sb_modes(VP9_COMP *cpi, const TileInfo *const tile, int mi_row, int mi_col, int *totalrate, int64_t *totaldist, BLOCK_SIZE bsize, PICK_MODE_CONTEXT *ctx, int64_t best_rd, int block)
{
    VP9_COMMON *const cm = &cpi->common;
    MACROBLOCK *const x = &cpi->mb;
    MACROBLOCKD *const xd = &x->e_mbd;
    MB_MODE_INFO *mbmi;
    struct macroblock_plane *const p = x->plane;
    struct macroblockd_plane *const pd = xd->plane;
    const AQ_MODE aq_mode = cpi->oxcf.aq_mode;
    int i, orig_rdmult;
    double rdmult_ratio;
    vp9_clear_system_state();
    rdmult_ratio = 1.0;
    x->use_lp32x32fdct = 1;
    if (bsize < BLOCK_8X8)
    {
        if (block != 0)
        {
            *totalrate = 0;
            *totaldist = 0;
            return;
        }
    }
    set_offsets(cpi, tile, mi_row, mi_col, bsize);
    mbmi = &xd->mi[0].src_mi->mbmi;
    mbmi->sb_type = bsize;
    for (i = 0; i < MAX_MB_PLANE; ++i)
    {
        p[i].coeff = ctx->coeff_pbuf[i][0];
        p[i].qcoeff = ctx->qcoeff_pbuf[i][0];
        pd[i].dqcoeff = ctx->dqcoeff_pbuf[i][0];
        p[i].eobs = ctx->eobs_pbuf[i][0];
    }
    ctx->is_coded = 0;
    ctx->skippable = 0;
    x->skip_recode = 0;
    mbmi->skip = 0;
    x->source_variance = get_sby_perpixel_variance(cpi, &x->plane[0].src, bsize);
    orig_rdmult = x->rdmult;
    if (aq_mode == VARIANCE_AQ)
    {
        const int energy = bsize <= BLOCK_16X16 ? x->mb_energy : vp9_block_energy(cpi, x, bsize);
        if (cm->frame_type == KEY_FRAME || cpi->refresh_alt_ref_frame || (cpi->refresh_golden_frame && !cpi->rc.is_src_frame_alt_ref))
        {
            mbmi->segment_id = vp9_vaq_segment_id(energy);
        }
        else
        {
            const uint8_t *const map = cm->seg.update_map ? cpi->segmentation_map : cm->last_frame_seg_map;
            mbmi->segment_id = vp9_get_segment_id(cm, map, bsize, mi_row, mi_col);
        }
        rdmult_ratio = vp9_vaq_rdmult_ratio(energy);
        vp9_init_plane_quantizers(cpi, x);
        vp9_clear_system_state();
        x->rdmult = (int)round(x->rdmult * rdmult_ratio);
    }
    else if (aq_mode == COMPLEXITY_AQ)
    {
        const int mi_offset = mi_row * cm->mi_cols + mi_col;
        unsigned char complexity = cpi->complexity_map[mi_offset];
        const int is_edge = (mi_row <= 1) || (mi_row >= (cm->mi_rows - 2)) || (mi_col <= 1) || (mi_col >= (cm->mi_cols - 2));
        if (!is_edge && (complexity > 128))
            x->rdmult += ((x->rdmult * (complexity - 128)) / 256);
    }
    else if (aq_mode == CYCLIC_REFRESH_AQ)
    {
        const uint8_t *const map = cm->seg.update_map ? cpi->segmentation_map : cm->last_frame_seg_map;
        if (vp9_get_segment_id(cm, map, bsize, mi_row, mi_col))
            x->rdmult = vp9_cyclic_refresh_get_rdmult(cpi->cyclic_refresh);
    }
    if (frame_is_intra_only(cm))
    {
        vp9_rd_pick_intra_mode_sb(cpi, x, totalrate, totaldist, bsize, ctx, best_rd);
    }
    else
    {
        if (bsize >= BLOCK_8X8)
        {
            if (vp9_segfeature_active(&cm->seg, mbmi->segment_id, SEG_LVL_SKIP))
                vp9_rd_pick_inter_mode_sb_seg_skip(cpi, x, totalrate, totaldist, bsize, ctx, best_rd);
            else
                vp9_rd_pick_inter_mode_sb(cpi, x, tile, mi_row, mi_col, totalrate, totaldist, bsize, ctx, best_rd);
        }
        else
        {
            vp9_rd_pick_inter_mode_sub8x8(cpi, x, tile, mi_row, mi_col, totalrate, totaldist, bsize, ctx, best_rd);
        }
    }
    x->rdmult = orig_rdmult;
    if (aq_mode == VARIANCE_AQ && *totalrate != INT_MAX)
    {
        vp9_clear_system_state();
        *totalrate = (int)round(*totalrate * rdmult_ratio);
    }
}