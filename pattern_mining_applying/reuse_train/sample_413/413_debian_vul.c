static int xan_decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    XanContext *s = avctx->priv_data;
    int ftype;
    int ret;
    s->pic.reference = 1;
    s->pic.buffer_hints = FF_BUFFER_HINTS_VALID | FF_BUFFER_HINTS_PRESERVE | FF_BUFFER_HINTS_REUSABLE;
    if ((ret = avctx->reget_buffer(avctx, &s->pic)))
    {
        av_log(s->avctx, AV_LOG_ERROR, "reget_buffer() failed\n");
        return ret;
    }
    bytestream2_init(&s->gb, avpkt->data, avpkt->size);
    ftype = bytestream2_get_le32(&s->gb);
    switch (ftype)
    {
    case 0:
        ret = xan_decode_frame_type0(avctx);
        break;
    case 1:
        ret = xan_decode_frame_type1(avctx);
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "Unknown frame type %d\n", ftype);
        return AVERROR_INVALIDDATA;
    }
    if (ret)
        return ret;
    *got_frame = 1;
    *(AVFrame *)data = s->pic;
    return avpkt->size;
}