static void inverse_transform_block(MACROBLOCKD *xd, int plane, int block, TX_SIZE tx_size, uint8_t *dst, int stride, int eob)
{
    struct macroblockd_plane *const pd = &xd->plane[plane];
    if (eob > 0)
    {
        TX_TYPE tx_type = DCT_DCT;
        tran_low_t *const dqcoeff = BLOCK_OFFSET(pd->dqcoeff, block);
        if (xd->lossless)
        {
            tx_type = DCT_DCT;
            vp9_iwht4x4_add(dqcoeff, dst, stride, eob);
        }
        else
        {
            const PLANE_TYPE plane_type = pd->plane_type;
            switch (tx_size)
            {
            case TX_4X4:
                tx_type = get_tx_type_4x4(plane_type, xd, block);
                vp9_iht4x4_add(tx_type, dqcoeff, dst, stride, eob);
                break;
            case TX_8X8:
                tx_type = get_tx_type(plane_type, xd);
                vp9_iht8x8_add(tx_type, dqcoeff, dst, stride, eob);
                break;
            case TX_16X16:
                tx_type = get_tx_type(plane_type, xd);
                vp9_iht16x16_add(tx_type, dqcoeff, dst, stride, eob);
                break;
            case TX_32X32:
                tx_type = DCT_DCT;
                vp9_idct32x32_add(dqcoeff, dst, stride, eob);
                break;
            default:
                assert(0 && "Invalid transform size");
            }
        }
        if (eob == 1)
        {
            vpx_memset(dqcoeff, 0, 2 * sizeof(dqcoeff[0]));
        }
        else
        {
            if (tx_type == DCT_DCT && tx_size <= TX_16X16 && eob <= 10)
                vpx_memset(dqcoeff, 0, 4 * (4 << tx_size) * sizeof(dqcoeff[0]));
            else if (tx_size == TX_32X32 && eob <= 34)
                vpx_memset(dqcoeff, 0, 256 * sizeof(dqcoeff[0]));
            else
                vpx_memset(dqcoeff, 0, (16 << (tx_size << 1)) * sizeof(dqcoeff[0]));
        }
    }
}