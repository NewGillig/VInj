static VP9_DENOISER_DECISION perform_motion_compensation(VP9_DENOISER *denoiser, MACROBLOCK *mb, BLOCK_SIZE bs, int increase_denoising, int mi_row, int mi_col, PICK_MODE_CONTEXT *ctx, int *motion_magnitude)
{
    int mv_col, mv_row;
    int sse_diff = ctx->zeromv_sse - ctx->newmv_sse;
    MV_REFERENCE_FRAME frame;
    MACROBLOCKD *filter_mbd = &mb->e_mbd;
    MB_MODE_INFO *mbmi = &filter_mbd->mi[0].src_mi->mbmi;
    MB_MODE_INFO saved_mbmi;
    int i, j;
    struct buf_2d saved_dst[MAX_MB_PLANE];
    struct buf_2d saved_pre[MAX_MB_PLANE][2];
    saved_mbmi = *mbmi;
    for (i = 0; i < MAX_MB_PLANE; ++i)
    {
        for (j = 0; j < 2; ++j)
        {
            saved_pre[i][j] = filter_mbd->plane[i].pre[j];
        }
        saved_dst[i] = filter_mbd->plane[i].dst;
    }
    mv_col = ctx->best_sse_mv.as_mv.col;
    mv_row = ctx->best_sse_mv.as_mv.row;
    *motion_magnitude = mv_row * mv_row + mv_col * mv_col;
    frame = ctx->best_reference_frame;
    if (frame != INTRA_FRAME && sse_diff > sse_diff_thresh(bs, increase_denoising, mv_row, mv_col))
    {
        mbmi->ref_frame[0] = ctx->best_reference_frame;
        mbmi->mode = ctx->best_sse_inter_mode;
        mbmi->mv[0] = ctx->best_sse_mv;
    }
    else
    {
        frame = ctx->best_zeromv_reference_frame;
        mbmi->ref_frame[0] = ctx->best_zeromv_reference_frame;
        mbmi->mode = ZEROMV;
        mbmi->mv[0].as_int = 0;
        ctx->best_sse_inter_mode = ZEROMV;
        ctx->best_sse_mv.as_int = 0;
        ctx->newmv_sse = ctx->zeromv_sse;
    }
    for (j = 0; j < 2; ++j)
    {
        filter_mbd->plane[0].pre[j].buf = block_start(denoiser->running_avg_y[frame].y_buffer, denoiser->running_avg_y[frame].y_stride, mi_row, mi_col);
        filter_mbd->plane[0].pre[j].stride = denoiser->running_avg_y[frame].y_stride;
        filter_mbd->plane[1].pre[j].buf = block_start(denoiser->running_avg_y[frame].u_buffer, denoiser->running_avg_y[frame].uv_stride, mi_row, mi_col);
        filter_mbd->plane[1].pre[j].stride = denoiser->running_avg_y[frame].uv_stride;
        filter_mbd->plane[2].pre[j].buf = block_start(denoiser->running_avg_y[frame].v_buffer, denoiser->running_avg_y[frame].uv_stride, mi_row, mi_col);
        filter_mbd->plane[2].pre[j].stride = denoiser->running_avg_y[frame].uv_stride;
    }
    filter_mbd->plane[0].dst.buf = block_start(denoiser->mc_running_avg_y.y_buffer, denoiser->mc_running_avg_y.y_stride, mi_row, mi_col);
    filter_mbd->plane[0].dst.stride = denoiser->mc_running_avg_y.y_stride;
    filter_mbd->plane[1].dst.buf = block_start(denoiser->mc_running_avg_y.u_buffer, denoiser->mc_running_avg_y.uv_stride, mi_row, mi_col);
    filter_mbd->plane[1].dst.stride = denoiser->mc_running_avg_y.uv_stride;
    filter_mbd->plane[2].dst.buf = block_start(denoiser->mc_running_avg_y.v_buffer, denoiser->mc_running_avg_y.uv_stride, mi_row, mi_col);
    filter_mbd->plane[2].dst.stride = denoiser->mc_running_avg_y.uv_stride;
    vp9_build_inter_predictors_sby(filter_mbd, mv_row, mv_col, bs);
    *mbmi = saved_mbmi;
    for (i = 0; i < MAX_MB_PLANE; ++i)
    {
        for (j = 0; j < 2; ++j)
        {
            filter_mbd->plane[i].pre[j] = saved_pre[i][j];
        }
        filter_mbd->plane[i].dst = saved_dst[i];
    }
    mv_row = ctx->best_sse_mv.as_mv.row;
    mv_col = ctx->best_sse_mv.as_mv.col;
    if (ctx->newmv_sse > sse_thresh(bs, increase_denoising))
    {
        return COPY_BLOCK;
    }
    if (mv_row * mv_row + mv_col * mv_col > *noise_motion_thresh(bs, increase_denoising))
    {
        return COPY_BLOCK;
    }
    return FILTER_BLOCK;
}