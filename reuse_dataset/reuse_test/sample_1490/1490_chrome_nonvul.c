static void encode_block_pass1(int plane, int block, BLOCK_SIZE plane_bsize, TX_SIZE tx_size, void *arg)
{
    MACROBLOCK *const x = (MACROBLOCK *)arg;
    MACROBLOCKD *const xd = &x->e_mbd;
    struct macroblock_plane *const p = &x->plane[plane];
    struct macroblockd_plane *const pd = &xd->plane[plane];
    tran_low_t *const dqcoeff = BLOCK_OFFSET(pd->dqcoeff, block);
    int i, j;
    uint8_t *dst;
    txfrm_block_to_raster_xy(plane_bsize, tx_size, block, &i, &j);
    dst = &pd->dst.buf[4 * j * pd->dst.stride + 4 * i];
    vp9_xform_quant(x, plane, block, plane_bsize, tx_size);
    if (p->eobs[block] > 0)
        x->itxm_add(dqcoeff, dst, pd->dst.stride, p->eobs[block]);
}