static void predict_and_reconstruct_intra_block(int plane, int block, BLOCK_SIZE plane_bsize, TX_SIZE tx_size, void *arg)
{
    struct intra_args *const args = (struct intra_args *)arg;
    VP9_COMMON *const cm = args->cm;
    MACROBLOCKD *const xd = args->xd;
    struct macroblockd_plane *const pd = &xd->plane[plane];
    MODE_INFO *const mi = xd->mi[0].src_mi;
    const PREDICTION_MODE mode = (plane == 0) ? get_y_mode(mi, block) : mi->mbmi.uv_mode;
    int x, y;
    uint8_t *dst;
    txfrm_block_to_raster_xy(plane_bsize, tx_size, block, &x, &y);
    dst = &pd->dst.buf[4 * y * pd->dst.stride + 4 * x];
    vp9_predict_intra_block(xd, block >> (tx_size << 1), b_width_log2(plane_bsize), tx_size, mode, dst, pd->dst.stride, dst, pd->dst.stride, x, y, plane);
    if (!mi->mbmi.skip)
    {
        const int eob = vp9_decode_block_tokens(cm, xd, plane, block, plane_bsize, x, y, tx_size, args->r);
        inverse_transform_block(xd, plane, block, tx_size, dst, pd->dst.stride, eob);
    }
}