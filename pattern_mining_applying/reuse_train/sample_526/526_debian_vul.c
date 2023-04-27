static int decode_slice(AVCodecContext *avctx, void *tdata)
{
    ProresThreadData *td = tdata;
    ProresContext *ctx = avctx->priv_data;
    int mb_x_pos = td->x_pos;
    int mb_y_pos = td->y_pos;
    int pic_num = ctx->pic_num;
    int slice_num = td->slice_num;
    int mbs_per_slice = td->slice_width;
    const uint8_t *buf;
    uint8_t *y_data, *u_data, *v_data;
    AVFrame *pic = avctx->coded_frame;
    int i, sf, slice_width_factor;
    int slice_data_size, hdr_size, y_data_size, u_data_size, v_data_size;
    int y_linesize, u_linesize, v_linesize;
    buf = ctx->slice_data[slice_num].index;
    slice_data_size = ctx->slice_data[slice_num + 1].index - buf;
    slice_width_factor = av_log2(mbs_per_slice);
    y_data = pic->data[0];
    u_data = pic->data[1];
    v_data = pic->data[2];
    y_linesize = pic->linesize[0];
    u_linesize = pic->linesize[1];
    v_linesize = pic->linesize[2];
    if (pic->interlaced_frame)
    {
        if (!(pic_num ^ pic->top_field_first))
        {
            y_data += y_linesize;
            u_data += u_linesize;
            v_data += v_linesize;
        }
        y_linesize <<= 1;
        u_linesize <<= 1;
        v_linesize <<= 1;
    }
    if (slice_data_size < 6)
    {
        av_log(avctx, AV_LOG_ERROR, "slice data too small\n");
        return AVERROR_INVALIDDATA;
    }
    hdr_size = buf[0] >> 3;
    y_data_size = AV_RB16(buf + 2);
    u_data_size = AV_RB16(buf + 4);
    v_data_size = hdr_size > 7 ? AV_RB16(buf + 6) : slice_data_size - y_data_size - u_data_size - hdr_size;
    if (hdr_size + y_data_size + u_data_size + v_data_size > slice_data_size || v_data_size < 0 || hdr_size < 6)
    {
        av_log(avctx, AV_LOG_ERROR, "invalid data size\n");
        return AVERROR_INVALIDDATA;
    }
    sf = av_clip(buf[1], 1, 224);
    sf = sf > 128 ? (sf - 96) << 2 : sf;
    if (ctx->qmat_changed || sf != td->prev_slice_sf)
    {
        td->prev_slice_sf = sf;
        for (i = 0; i < 64; i++)
        {
            td->qmat_luma_scaled[ctx->dsp.idct_permutation[i]] = ctx->qmat_luma[i] * sf;
            td->qmat_chroma_scaled[ctx->dsp.idct_permutation[i]] = ctx->qmat_chroma[i] * sf;
        }
    }
    decode_slice_plane(ctx, td, buf + hdr_size, y_data_size, (uint16_t *)(y_data + (mb_y_pos << 4) * y_linesize + (mb_x_pos << 5)), y_linesize, mbs_per_slice, 4, slice_width_factor + 2, td->qmat_luma_scaled, 0);
    decode_slice_plane(ctx, td, buf + hdr_size + y_data_size, u_data_size, (uint16_t *)(u_data + (mb_y_pos << 4) * u_linesize + (mb_x_pos << ctx->mb_chroma_factor)), u_linesize, mbs_per_slice, ctx->num_chroma_blocks, slice_width_factor + ctx->chroma_factor - 1, td->qmat_chroma_scaled, 1);
    decode_slice_plane(ctx, td, buf + hdr_size + y_data_size + u_data_size, v_data_size, (uint16_t *)(v_data + (mb_y_pos << 4) * v_linesize + (mb_x_pos << ctx->mb_chroma_factor)), v_linesize, mbs_per_slice, ctx->num_chroma_blocks, slice_width_factor + ctx->chroma_factor - 1, td->qmat_chroma_scaled, 1);
    return 0;
}