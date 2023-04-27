static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    int h, w, ret;
    AVFrame *pic = data;
    const uint8_t *psrc = avpkt->data;
    uint16_t *y, *u, *v;
    int aligned_width = ((avctx->width + 47) / 48) * 48;
    int stride = aligned_width * 8 / 3;
    if (avpkt->size < stride * avctx->height)
    {
        av_log(avctx, AV_LOG_ERROR, "packet too small\n");
        return AVERROR_INVALIDDATA;
    }
    if ((ret = ff_get_buffer(avctx, pic, 0)) < 0)
        return ret;
    y = (uint16_t *)pic->data[0];
    u = (uint16_t *)pic->data[1];
    v = (uint16_t *)pic->data[2];
    pic->pict_type = AV_PICTURE_TYPE_I;
    pic->key_frame = 1;
    val = av_le2ne32(*src++);
    *a++ = val & 0x3FF;
    *b++ = (val >> 10) & 0x3FF;
    *c++ = (val >> 20) & 0x3FF;
}