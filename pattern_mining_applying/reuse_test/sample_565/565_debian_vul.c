static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    int field, ret;
    AVFrame *pic = avctx->coded_frame;
    const uint8_t *buf = avpkt->data;
    const uint8_t *buf_end = buf + avpkt->size;
    if (pic->data[0])
        avctx->release_buffer(avctx, pic);
    if (avpkt->size < avctx->width * 2 * avctx->height + 4 + 2 * 8)
    {
        av_log(avctx, AV_LOG_ERROR, "Packet is too small.\n");
        return AVERROR_INVALIDDATA;
    }
    if (bytestream_get_le32(&buf) != MKTAG('F', 'R', 'W', '1'))
    {
        av_log(avctx, AV_LOG_ERROR, "incorrect marker\n");
        return AVERROR_INVALIDDATA;
    }
    pic->reference = 0;
    if ((ret = ff_get_buffer(avctx, pic)) < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed\n");
        return ret;
    }
    pic->pict_type = AV_PICTURE_TYPE_I;
    pic->key_frame = 1;
    pic->interlaced_frame = 1;
    pic->top_field_first = 1;
    for (field = 0; field < 2; field++)
    {
        int i;
        int field_h = (avctx->height + !field) >> 1;
        int field_size, min_field_size = avctx->width * 2 * field_h;
        uint8_t *dst = pic->data[0];
        if (buf_end - buf < 8)
            return AVERROR_INVALIDDATA;
        buf += 4;
        field_size = bytestream_get_le32(&buf);
        if (field_size < min_field_size)
        {
            av_log(avctx, AV_LOG_ERROR, "Field size %i is too small (required %i)\n", field_size, min_field_size);
            return AVERROR_INVALIDDATA;
        }
        if (buf_end - buf < field_size)
        {
            av_log(avctx, AV_LOG_ERROR, "Packet is too small, need %i, have %i\n", field_size, (int)(buf_end - buf));
            return AVERROR_INVALIDDATA;
        }
        if (field)
            dst += pic->linesize[0];
        for (i = 0; i < field_h; i++)
        {
            memcpy(dst, buf, avctx->width * 2);
            buf += avctx->width * 2;
            dst += pic->linesize[0] << 1;
        }
        buf += field_size - min_field_size;
    }
    *got_frame = 1;
    *(AVFrame *)data = *pic;
    return avpkt->size;
}