static int find_best_16x16_intra(VP9_COMP *cpi, PREDICTION_MODE *pbest_mode)
{
    MACROBLOCK *const x = &cpi->mb;
    MACROBLOCKD *const xd = &x->e_mbd;
    PREDICTION_MODE best_mode = -1, mode;
    unsigned int best_err = INT_MAX;
    for (mode = DC_PRED; mode <= TM_PRED; mode++)
    {
        unsigned int err;
        xd->mi[0].src_mi->mbmi.mode = mode;
        vp9_predict_intra_block(xd, 0, 2, TX_16X16, mode, x->plane[0].src.buf, x->plane[0].src.stride, xd->plane[0].dst.buf, xd->plane[0].dst.stride, 0, 0, 0);
        err = vp9_sad16x16(x->plane[0].src.buf, x->plane[0].src.stride, xd->plane[0].dst.buf, xd->plane[0].dst.stride);
        if (err < best_err)
        {
            best_err = err;
            best_mode = mode;
        }
    }
    if (pbest_mode)
        *pbest_mode = best_mode;
    return best_err;
}