static void encode_block(int plane, int block, BLOCK_SIZE plane_bsize, TX_SIZE tx_size, void *arg)
{
    struct encode_b_args *const args = arg;
    MACROBLOCK *const x = args->x;
    MACROBLOCKD *const xd = &x->e_mbd;
    struct optimize_ctx *const ctx = args->ctx;
    struct macroblock_plane *const p = &x->plane[plane];
    struct macroblockd_plane *const pd = &xd->plane[plane];
    tran_low_t *const dqcoeff = BLOCK_OFFSET(pd->dqcoeff, block);
    int i, j;
    uint8_t *dst;
    ENTROPY_CONTEXT *a, *l;
    txfrm_block_to_raster_xy(plane_bsize, tx_size, block, &i, &j);
    dst = &pd->dst.buf[4 * j * pd->dst.stride + 4 * i];
    a = &ctx->ta[plane][i];
    l = &ctx->tl[plane][j];
    if (x->zcoeff_blk[tx_size][block] && plane == 0)
    {
        p->eobs[block] = 0;
        *a = *l = 0;
        return;
    }
    if (!x->skip_recode)
    {
        if (max_txsize_lookup[plane_bsize] == tx_size)
        {
            if (x->skip_txfm[(plane << 2) + (block >> (tx_size << 1))] == 0)
            {
                if (x->quant_fp)
                    vp9_xform_quant_fp(x, plane, block, plane_bsize, tx_size);
                else
                    vp9_xform_quant(x, plane, block, plane_bsize, tx_size);
            }
            else if (x->skip_txfm[(plane << 2) + (block >> (tx_size << 1))] == 2)
            {
                vp9_xform_quant_dc(x, plane, block, plane_bsize, tx_size);
            }
            else
            {
                p->eobs[block] = 0;
                *a = *l = 0;
                return;
            }
        }
        else
        {
            vp9_xform_quant(x, plane, block, plane_bsize, tx_size);
        }
    }
    if (x->optimize && (!x->skip_recode || !x->skip_optimize))
    {
        const int ctx = combine_entropy_contexts(*a, *l);
        *a = *l = optimize_b(x, plane, block, tx_size, ctx) > 0;
    }
    else
    {
        *a = *l = p->eobs[block] > 0;
    }
    if (p->eobs[block])
        *(args->skip) = 0;
    if (x->skip_encode || p->eobs[block] == 0)
        return;
    switch (tx_size)
    {
    case TX_32X32:
        vp9_idct32x32_add(dqcoeff, dst, pd->dst.stride, p->eobs[block]);
        break;
    case TX_16X16:
        vp9_idct16x16_add(dqcoeff, dst, pd->dst.stride, p->eobs[block]);
        break;
    case TX_8X8:
        vp9_idct8x8_add(dqcoeff, dst, pd->dst.stride, p->eobs[block]);
        break;
    case TX_4X4:
        x->itxm_add(dqcoeff, dst, pd->dst.stride, p->eobs[block]);
        break;
    default:
        assert(0 && "Invalid transform size");
        break;
    }
}