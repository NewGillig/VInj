void ff_xvmc_decode_mb(MpegEncContext *s)
{
    XvMCMacroBlock *mv_block;
    struct xvmc_pix_fmt *render;
    int i, cbp, blocks_per_mb;
    const int mb_xy = s->mb_y * s->mb_stride + s->mb_x;
    if (s->encoding)
    {
        av_log(s->avctx, AV_LOG_ERROR, "XVMC doesn't support encoding!!!\n");
        return;
    }
    if (!s->mb_intra)
    {
        s->last_dc[0] = s->last_dc[1] = s->last_dc[2] = 128 << s->intra_dc_precision;
    }
    s->mb_skipped = 0;
    s->current_picture.f.qscale_table[mb_xy] = s->qscale;
    render = (struct xvmc_pix_fmt *)s->current_picture.f.data[2];
    assert(render);
    assert(render->xvmc_id == AV_XVMC_ID);
    assert(render->mv_blocks);
    mv_block = &render->mv_blocks[render->start_mv_blocks_num + render->filled_mv_blocks_num];
    mv_block->x = s->mb_x;
    mv_block->y = s->mb_y;
    mv_block->dct_type = s->interlaced_dct;
    if (s->mb_intra)
    {
        mv_block->macroblock_type = XVMC_MB_TYPE_INTRA;
    }
    else
    {
        mv_block->macroblock_type = XVMC_MB_TYPE_PATTERN;
        if (s->mv_dir & MV_DIR_FORWARD)
        {
            mv_block->macroblock_type |= XVMC_MB_TYPE_MOTION_FORWARD;
            mv_block->PMV[0][0][0] = s->mv[0][0][0];
            mv_block->PMV[0][0][1] = s->mv[0][0][1];
            mv_block->PMV[1][0][0] = s->mv[0][1][0];
            mv_block->PMV[1][0][1] = s->mv[0][1][1];
        }
        if (s->mv_dir & MV_DIR_BACKWARD)
        {
            mv_block->macroblock_type |= XVMC_MB_TYPE_MOTION_BACKWARD;
            mv_block->PMV[0][1][0] = s->mv[1][0][0];
            mv_block->PMV[0][1][1] = s->mv[1][0][1];
            mv_block->PMV[1][1][0] = s->mv[1][1][0];
            mv_block->PMV[1][1][1] = s->mv[1][1][1];
        }
        switch (s->mv_type)
        {
        case MV_TYPE_16X16:
            mv_block->motion_type = XVMC_PREDICTION_FRAME;
            break;
        case MV_TYPE_16X8:
            mv_block->motion_type = XVMC_PREDICTION_16x8;
            break;
        case MV_TYPE_FIELD:
            mv_block->motion_type = XVMC_PREDICTION_FIELD;
            if (s->picture_structure == PICT_FRAME)
            {
                mv_block->PMV[0][0][1] <<= 1;
                mv_block->PMV[1][0][1] <<= 1;
                mv_block->PMV[0][1][1] <<= 1;
                mv_block->PMV[1][1][1] <<= 1;
            }
            break;
        case MV_TYPE_DMV:
            mv_block->motion_type = XVMC_PREDICTION_DUAL_PRIME;
            if (s->picture_structure == PICT_FRAME)
            {
                mv_block->PMV[0][0][0] = s->mv[0][0][0];
                mv_block->PMV[0][0][1] = s->mv[0][0][1] << 1;
                mv_block->PMV[0][1][0] = s->mv[0][0][0];
                mv_block->PMV[0][1][1] = s->mv[0][0][1] << 1;
                mv_block->PMV[1][0][0] = s->mv[0][2][0];
                mv_block->PMV[1][0][1] = s->mv[0][2][1] << 1;
                mv_block->PMV[1][1][0] = s->mv[0][3][0];
                mv_block->PMV[1][1][1] = s->mv[0][3][1] << 1;
            }
            else
            {
                mv_block->PMV[0][1][0] = s->mv[0][2][0];
                mv_block->PMV[0][1][1] = s->mv[0][2][1];
            }
            break;
        default:
            assert(0);
        }
        mv_block->motion_vertical_field_select = 0;
        if (s->mv_type == MV_TYPE_FIELD || s->mv_type == MV_TYPE_16X8)
        {
            mv_block->motion_vertical_field_select |= s->field_select[0][0];
            mv_block->motion_vertical_field_select |= s->field_select[1][0] << 1;
            mv_block->motion_vertical_field_select |= s->field_select[0][1] << 2;
            mv_block->motion_vertical_field_select |= s->field_select[1][1] << 3;
        }
    }
    mv_block->index = render->next_free_data_block_num;
    blocks_per_mb = 6;
    if (s->chroma_format >= 2)
    {
        blocks_per_mb = 4 + (1 << s->chroma_format);
    }
    cbp = 0;
    for (i = 0; i < blocks_per_mb; i++)
    {
        cbp += cbp;
        if (s->block_last_index[i] >= 0)
            cbp++;
    }
    if (s->flags & CODEC_FLAG_GRAY)
    {
        if (s->mb_intra)
        {
            for (i = 4; i < blocks_per_mb; i++)
            {
                memset(s->pblocks[i], 0, sizeof(*s->pblocks[i]));
                if (!render->unsigned_intra)
                    *s->pblocks[i][0] = 1 << 10;
            }
        }
        else
        {
            cbp &= 0xf << (blocks_per_mb - 4);
            blocks_per_mb = 4;
        }
    }
    mv_block->coded_block_pattern = cbp;
    if (cbp == 0)
        mv_block->macroblock_type &= ~XVMC_MB_TYPE_PATTERN;
    for (i = 0; i < blocks_per_mb; i++)
    {
        if (s->block_last_index[i] >= 0)
        {
            if (s->mb_intra && (render->idct || !render->unsigned_intra))
                *s->pblocks[i][0] -= 1 << 10;
            if (!render->idct)
            {
                s->dsp.idct(*s->pblocks[i]);
            }
            if (s->avctx->xvmc_acceleration == 1)
            {
                memcpy(&render->data_blocks[render->next_free_data_block_num * 64], s->pblocks[i], sizeof(*s->pblocks[i]));
            }
            render->next_free_data_block_num++;
        }
    }
    render->filled_mv_blocks_num++;
    assert(render->filled_mv_blocks_num <= render->allocated_mv_blocks);
    assert(render->next_free_data_block_num <= render->allocated_data_blocks);
    if (render->filled_mv_blocks_num == render->allocated_mv_blocks)
        ff_mpeg_draw_horiz_band(s, 0, 0);
}