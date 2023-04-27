static void encode_breakout_test(VP9_COMP *cpi, MACROBLOCK *x, BLOCK_SIZE bsize, int mi_row, int mi_col, MV_REFERENCE_FRAME ref_frame, PREDICTION_MODE this_mode, unsigned int var_y, unsigned int sse_y, struct buf_2d yv12_mb[][MAX_MB_PLANE], int *rate, int64_t *dist)
{
    MACROBLOCKD *xd = &x->e_mbd;
    MB_MODE_INFO *mbmi = &xd->mi[0].src_mi->mbmi;
    const BLOCK_SIZE uv_size = get_plane_block_size(bsize, &xd->plane[1]);
    unsigned int var = var_y, sse = sse_y;
    unsigned int thresh_ac;
    unsigned int thresh_dc;
    if (x->encode_breakout > 0)
    {
        const unsigned int max_thresh = 36000;
        const unsigned int min_thresh = MIN(((unsigned int)x->encode_breakout << 4), max_thresh);
        thresh_ac = (xd->plane[0].dequant[1] * xd->plane[0].dequant[1]) / 9;
        thresh_ac = clamp(thresh_ac, min_thresh, max_thresh);
        thresh_ac >>= -(b_width_log2(bsize) + b_height_log2(bsize));
        thresh_dc = (xd->plane[0].dequant[0] * xd->plane[0].dequant[0] >> 6);
    }
    else
    {
        thresh_ac = 0;
        thresh_dc = 0;
    }
    if (var <= thresh_ac && (sse - var) <= thresh_dc)
    {
        unsigned int sse_u, sse_v;
        unsigned int var_u, var_v;
        if (x->encode_breakout == 0)
        {
            xd->plane[1].pre[0] = yv12_mb[ref_frame][1];
            xd->plane[2].pre[0] = yv12_mb[ref_frame][2];
            vp9_build_inter_predictors_sbuv(xd, mi_row, mi_col, bsize);
        }
        var_u = cpi->fn_ptr[uv_size].vf(x->plane[1].src.buf, x->plane[1].src.stride, xd->plane[1].dst.buf, xd->plane[1].dst.stride, &sse_u);
        if ((var_u * 4 <= thresh_ac) && (sse_u - var_u <= thresh_dc))
        {
            var_v = cpi->fn_ptr[uv_size].vf(x->plane[2].src.buf, x->plane[2].src.stride, xd->plane[2].dst.buf, xd->plane[2].dst.stride, &sse_v);
            if ((var_v * 4 <= thresh_ac) && (sse_v - var_v <= thresh_dc))
            {
                x->skip = 1;
                *rate = cpi->inter_mode_cost[mbmi->mode_context[ref_frame]][INTER_OFFSET(this_mode)];
                *dist = (sse << 4);
            }
        }
    }
}