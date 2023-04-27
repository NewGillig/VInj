static int rpza_decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size = avpkt->size;
    RpzaContext *s = avctx->priv_data;
    int ret;
    s->buf = buf;
    s->size = buf_size;
    s->frame.reference = 1;
    s->frame.buffer_hints = FF_BUFFER_HINTS_VALID | FF_BUFFER_HINTS_PRESERVE | FF_BUFFER_HINTS_REUSABLE;
    if ((ret = avctx->reget_buffer(avctx, &s->frame)) < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "reget_buffer() failed\n");
        return ret;
    }
    rpza_decode_stream(s);
    *got_frame = 1;
    *(AVFrame *)data = s->frame;
    return buf_size;
}