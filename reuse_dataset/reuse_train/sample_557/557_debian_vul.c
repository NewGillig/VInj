static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    LOCOContext *const l = avctx->priv_data;
    const uint8_t *buf = avpkt->data;
    int buf_size = avpkt->size;
    AVFrame *const p = &l->pic;
    int decoded, ret;
    if (p->data[0])
        avctx->release_buffer(avctx, p);
    p->reference = 0;
    if ((ret = ff_get_buffer(avctx, p)) < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed\n");
        return ret;
    }
    p->key_frame = 1;
    switch (l->mode)
    {
    case LOCO_CYUY2:
    case LOCO_YUY2:
    case LOCO_UYVY:
        decoded = loco_decode_plane(l, p->data[0], avctx->width, avctx->height, p->linesize[0], buf, buf_size, 1);
        if (decoded >= buf_size)
            goto buf_too_small;
        buf += decoded;
        buf_size -= decoded;
        decoded = loco_decode_plane(l, p->data[1], avctx->width / 2, avctx->height, p->linesize[1], buf, buf_size, 1);
        if (decoded >= buf_size)
            goto buf_too_small;
        buf += decoded;
        buf_size -= decoded;
        decoded = loco_decode_plane(l, p->data[2], avctx->width / 2, avctx->height, p->linesize[2], buf, buf_size, 1);
        break;
    case LOCO_CYV12:
    case LOCO_YV12:
        decoded = loco_decode_plane(l, p->data[0], avctx->width, avctx->height, p->linesize[0], buf, buf_size, 1);
        if (decoded >= buf_size)
            goto buf_too_small;
        buf += decoded;
        buf_size -= decoded;
        decoded = loco_decode_plane(l, p->data[2], avctx->width / 2, avctx->height / 2, p->linesize[2], buf, buf_size, 1);
        if (decoded >= buf_size)
            goto buf_too_small;
        buf += decoded;
        buf_size -= decoded;
        decoded = loco_decode_plane(l, p->data[1], avctx->width / 2, avctx->height / 2, p->linesize[1], buf, buf_size, 1);
        break;
    case LOCO_CRGB:
    case LOCO_RGB:
        decoded = loco_decode_plane(l, p->data[0] + p->linesize[0] * (avctx->height - 1), avctx->width, avctx->height, -p->linesize[0], buf, buf_size, 3);
        if (decoded >= buf_size)
            goto buf_too_small;
        buf += decoded;
        buf_size -= decoded;
        decoded = loco_decode_plane(l, p->data[0] + p->linesize[0] * (avctx->height - 1) + 1, avctx->width, avctx->height, -p->linesize[0], buf, buf_size, 3);
        if (decoded >= buf_size)
            goto buf_too_small;
        buf += decoded;
        buf_size -= decoded;
        decoded = loco_decode_plane(l, p->data[0] + p->linesize[0] * (avctx->height - 1) + 2, avctx->width, avctx->height, -p->linesize[0], buf, buf_size, 3);
        break;
    case LOCO_RGBA:
        decoded = loco_decode_plane(l, p->data[0], avctx->width, avctx->height, p->linesize[0], buf, buf_size, 4);
        if (decoded >= buf_size)
            goto buf_too_small;
        buf += decoded;
        buf_size -= decoded;
        decoded = loco_decode_plane(l, p->data[0] + 1, avctx->width, avctx->height, p->linesize[0], buf, buf_size, 4);
        if (decoded >= buf_size)
            goto buf_too_small;
        buf += decoded;
        buf_size -= decoded;
        decoded = loco_decode_plane(l, p->data[0] + 2, avctx->width, avctx->height, p->linesize[0], buf, buf_size, 4);
        if (decoded >= buf_size)
            goto buf_too_small;
        buf += decoded;
        buf_size -= decoded;
        decoded = loco_decode_plane(l, p->data[0] + 3, avctx->width, avctx->height, p->linesize[0], buf, buf_size, 4);
        break;
    }
    *got_frame = 1;
    *(AVFrame *)data = l->pic;
    return buf_size;
buf_too_small:
    av_log(avctx, AV_LOG_ERROR, "Input data too small.\n");
    return AVERROR(EINVAL);
}