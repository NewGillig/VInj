static void MCFUNC(hl_motion)(H264Context *h, uint8_t *dest_y, uint8_t *dest_cb, uint8_t *dest_cr, qpel_mc_func (*qpix_put)[16], h264_chroma_mc_func(*chroma_put), qpel_mc_func (*qpix_avg)[16], h264_chroma_mc_func(*chroma_avg), h264_weight_func *weight_op, h264_biweight_func *weight_avg)
{
    const int mb_xy = h->mb_xy;
    const int mb_type = h->cur_pic.f.mb_type[mb_xy];
    assert(IS_INTER(mb_type));
    if (HAVE_THREADS && (h->avctx->active_thread_type & FF_THREAD_FRAME))
        await_references(h);
    prefetch_motion(h, 0, PIXEL_SHIFT, CHROMA_IDC);
    if (IS_16X16(mb_type))
    {
        mc_part(h, 0, 1, 16, 0, dest_y, dest_cb, dest_cr, 0, 0, qpix_put[0], chroma_put[0], qpix_avg[0], chroma_avg[0], weight_op, weight_avg, IS_DIR(mb_type, 0, 0), IS_DIR(mb_type, 0, 1));
    }
    else if (IS_16X8(mb_type))
    {
        mc_part(h, 0, 0, 8, 8 << PIXEL_SHIFT, dest_y, dest_cb, dest_cr, 0, 0, qpix_put[1], chroma_put[0], qpix_avg[1], chroma_avg[0], weight_op, weight_avg, IS_DIR(mb_type, 0, 0), IS_DIR(mb_type, 0, 1));
        mc_part(h, 8, 0, 8, 8 << PIXEL_SHIFT, dest_y, dest_cb, dest_cr, 0, 4, qpix_put[1], chroma_put[0], qpix_avg[1], chroma_avg[0], weight_op, weight_avg, IS_DIR(mb_type, 1, 0), IS_DIR(mb_type, 1, 1));
    }
    else if (IS_8X16(mb_type))
    {
        mc_part(h, 0, 0, 16, 8 * h->mb_linesize, dest_y, dest_cb, dest_cr, 0, 0, qpix_put[1], chroma_put[1], qpix_avg[1], chroma_avg[1], &weight_op[1], &weight_avg[1], IS_DIR(mb_type, 0, 0), IS_DIR(mb_type, 0, 1));
        mc_part(h, 4, 0, 16, 8 * h->mb_linesize, dest_y, dest_cb, dest_cr, 4, 0, qpix_put[1], chroma_put[1], qpix_avg[1], chroma_avg[1], &weight_op[1], &weight_avg[1], IS_DIR(mb_type, 1, 0), IS_DIR(mb_type, 1, 1));
    }
    else
    {
        int i;
        assert(IS_8X8(mb_type));
        for (i = 0; i < 4; i++)
        {
            const int sub_mb_type = h->sub_mb_type[i];
            const int n = 4 * i;
            int x_offset = (i & 1) << 2;
            int y_offset = (i & 2) << 1;
            if (IS_SUB_8X8(sub_mb_type))
            {
                mc_part(h, n, 1, 8, 0, dest_y, dest_cb, dest_cr, x_offset, y_offset, qpix_put[1], chroma_put[1], qpix_avg[1], chroma_avg[1], &weight_op[1], &weight_avg[1], IS_DIR(sub_mb_type, 0, 0), IS_DIR(sub_mb_type, 0, 1));
            }
            else if (IS_SUB_8X4(sub_mb_type))
            {
                mc_part(h, n, 0, 4, 4 << PIXEL_SHIFT, dest_y, dest_cb, dest_cr, x_offset, y_offset, qpix_put[2], chroma_put[1], qpix_avg[2], chroma_avg[1], &weight_op[1], &weight_avg[1], IS_DIR(sub_mb_type, 0, 0), IS_DIR(sub_mb_type, 0, 1));
                mc_part(h, n + 2, 0, 4, 4 << PIXEL_SHIFT, dest_y, dest_cb, dest_cr, x_offset, y_offset + 2, qpix_put[2], chroma_put[1], qpix_avg[2], chroma_avg[1], &weight_op[1], &weight_avg[1], IS_DIR(sub_mb_type, 0, 0), IS_DIR(sub_mb_type, 0, 1));
            }
            else if (IS_SUB_4X8(sub_mb_type))
            {
                mc_part(h, n, 0, 8, 4 * h->mb_linesize, dest_y, dest_cb, dest_cr, x_offset, y_offset, qpix_put[2], chroma_put[2], qpix_avg[2], chroma_avg[2], &weight_op[2], &weight_avg[2], IS_DIR(sub_mb_type, 0, 0), IS_DIR(sub_mb_type, 0, 1));
                mc_part(h, n + 1, 0, 8, 4 * h->mb_linesize, dest_y, dest_cb, dest_cr, x_offset + 2, y_offset, qpix_put[2], chroma_put[2], qpix_avg[2], chroma_avg[2], &weight_op[2], &weight_avg[2], IS_DIR(sub_mb_type, 0, 0), IS_DIR(sub_mb_type, 0, 1));
            }
            else
            {
                int j;
                assert(IS_SUB_4X4(sub_mb_type));
                for (j = 0; j < 4; j++)
                {
                    int sub_x_offset = x_offset + 2 * (j & 1);
                    int sub_y_offset = y_offset + (j & 2);
                    mc_part(h, n + j, 1, 4, 0, dest_y, dest_cb, dest_cr, sub_x_offset, sub_y_offset, qpix_put[2], chroma_put[2], qpix_avg[2], chroma_avg[2], &weight_op[2], &weight_avg[2], IS_DIR(sub_mb_type, 0, 0), IS_DIR(sub_mb_type, 0, 1));
                }
            }
        }
    }
    prefetch_motion(h, 1, PIXEL_SHIFT, CHROMA_IDC);
}