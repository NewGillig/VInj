static void temporal_filter_predictors_mb_c(MACROBLOCKD *xd, uint8_t *y_mb_ptr, uint8_t *u_mb_ptr, uint8_t *v_mb_ptr, int stride, int uv_block_width, int uv_block_height, int mv_row, int mv_col, uint8_t *pred, struct scale_factors *scale, int x, int y)
{
    const int which_mv = 0;
    const MV mv = {mv_row, mv_col};
    const InterpKernel *const kernel = vp9_get_interp_kernel(xd->mi[0].src_mi->mbmi.interp_filter);
    enum mv_precision mv_precision_uv;
    int uv_stride;
    if (uv_block_width == 8)
    {
        uv_stride = (stride + 1) >> 1;
        mv_precision_uv = MV_PRECISION_Q4;
    }
    else
    {
        uv_stride = stride;
        mv_precision_uv = MV_PRECISION_Q3;
    }
    vp9_build_inter_predictor(y_mb_ptr, stride, &pred[0], 16, &mv, scale, 16, 16, which_mv, kernel, MV_PRECISION_Q3, x, y);
    vp9_build_inter_predictor(u_mb_ptr, uv_stride, &pred[256], uv_block_width, &mv, scale, uv_block_width, uv_block_height, which_mv, kernel, mv_precision_uv, x, y);
    vp9_build_inter_predictor(v_mb_ptr, uv_stride, &pred[512], uv_block_width, &mv, scale, uv_block_width, uv_block_height, which_mv, kernel, mv_precision_uv, x, y);
}