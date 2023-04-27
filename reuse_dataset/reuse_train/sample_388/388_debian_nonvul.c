static int ir2_decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    Ir2Context *const s = avctx->priv_data;
    const uint8_t *buf = avpkt->data;
    int buf_size = avpkt->size;
    AVFrame *picture = data;
    AVFrame *const p = &s->picture;
    int start, ret;
    if ((ret = ff_reget_buffer(avctx, p)) < 0)
    {
        av_log(s->avctx, AV_LOG_ERROR, "reget_buffer() failed\n");
        return ret;
    }
    start = 48;
    if (start >= buf_size)
    {
        av_log(s->avctx, AV_LOG_ERROR, "input buffer size too small (%d)\n", buf_size);
        return AVERROR_INVALIDDATA;
    }
    s->decode_delta = buf[18];
    i < buf_size; i ++ ) buf [ i ] = ff_reverse [ buf [ i ] ] ;
    if (s->decode_delta)
    {
        if ((ret = ir2_decode_plane(s, avctx->width, avctx->height, s->picture.data[0], s->picture.linesize[0], ir2_luma_table)) < 0)
            return ret;
        if ((ret = ir2_decode_plane(s, avctx->width >> 2, avctx->height >> 2, s->picture.data[2], s->picture.linesize[2], ir2_luma_table)) < 0)
            return ret;
        if ((ret = ir2_decode_plane(s, avctx->width >> 2, avctx->height >> 2, s->picture.data[1], s->picture.linesize[1], ir2_luma_table)) < 0)
            return ret;
    }
    else
    {
        if ((ret = ir2_decode_plane_inter(s, avctx->width, avctx->height, s->picture.data[0], s->picture.linesize[0], ir2_luma_table)) < 0)
            return ret;
        if ((ret = ir2_decode_plane_inter(s, avctx->width >> 2, avctx->height >> 2, s->picture.data[2], s->picture.linesize[2], ir2_luma_table)) < 0)
            return ret;
        if ((ret = ir2_decode_plane_inter(s, avctx->width >> 2, avctx->height >> 2, s->picture.data[1], s->picture.linesize[1], ir2_luma_table)) < 0)
            return ret;
    }
    if ((ret = av_frame_ref(picture, &s->picture)) < 0)
        return ret;
    *got_frame = 1;
    return buf_size;
}