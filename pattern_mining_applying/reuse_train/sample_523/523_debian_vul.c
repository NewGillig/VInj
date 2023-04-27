static int truemotion1_decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int ret, buf_size = avpkt->size;
    TrueMotion1Context *s = avctx->priv_data;
    s->buf = buf;
    s->size = buf_size;
    if ((ret = truemotion1_decode_header(s)) < 0)
        return ret;
    s->frame.reference = 1;
    s->frame.buffer_hints = FF_BUFFER_HINTS_VALID | FF_BUFFER_HINTS_PRESERVE | FF_BUFFER_HINTS_REUSABLE;
    if ((ret = avctx->reget_buffer(avctx, &s->frame)) < 0)
    {
        av_log(s->avctx, AV_LOG_ERROR, "get_buffer() failed\n");
        return ret;
    }
    if (compression_types[s->compression].algorithm == ALGO_RGB24H)
    {
        truemotion1_decode_24bit(s);
    }
    else if (compression_types[s->compression].algorithm != ALGO_NOP)
    {
        truemotion1_decode_16bit(s);
    }
    *got_frame = 1;
    *(AVFrame *)data = s->frame;
    return buf_size;
}