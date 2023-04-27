static int ptx_decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    const uint8_t *buf_end = avpkt->data + avpkt->size;
    PTXContext *const s = avctx->priv_data;
    AVFrame *picture = data;
    AVFrame *const p = &s->picture;
    unsigned int offset, w, h, y, stride, bytes_per_pixel;
    int ret;
    uint8_t *ptr;
    if (buf_end - buf < 14)
        return AVERROR_INVALIDDATA;
    offset = AV_RL16(buf);
    w = AV_RL16(buf + 8);
    h = AV_RL16(buf + 10);
    bytes_per_pixel = AV_RL16(buf + 12) >> 3;
    if (bytes_per_pixel != 2)
    {
        av_log_ask_for_sample(avctx, "Image format is not RGB15.\n");
        return AVERROR_PATCHWELCOME;
    }
    avctx->pix_fmt = AV_PIX_FMT_RGB555;
    if (buf_end - buf < offset)
        return AVERROR_INVALIDDATA;
    if (offset != 0x2c)
        av_log_ask_for_sample(avctx, "offset != 0x2c\n");
    buf += offset;
    if (p->data[0])
        avctx->release_buffer(avctx, p);
    if ((ret = av_image_check_size(w, h, 0, avctx)) < 0)
        return ret;
    if (w != avctx->width || h != avctx->height)
        avcodec_set_dimensions(avctx, w, h);
    if ((ret = ff_get_buffer(avctx, p)) < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed\n");
        return ret;
    }
    p->pict_type = AV_PICTURE_TYPE_I;
    ptr = p->data[0];
    stride = p->linesize[0];
    for (y = 0; y < h && buf_end - buf >= w * bytes_per_pixel; y++)
    {
        for (x = 0; x < w * bytes_per_pixel; x += bytes_per_pixel)
            AV_WN16(ptr + x, AV_RL16(buf + x));
        buf += w * bytes_per_pixel;
    }
    *picture = s->picture;
    *got_frame = 1;
    if (y < h)
    {
        av_log(avctx, AV_LOG_WARNING, "incomplete packet\n");
        return avpkt->size;
    }
    return offset + w * h * bytes_per_pixel;
}