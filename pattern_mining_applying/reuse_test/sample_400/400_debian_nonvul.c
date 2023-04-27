static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    int h, w;
    AVFrame *pic = data;
    const uint8_t *src = avpkt->data;
    uint8_t *Y1, *Y2, *U, *V;
    int ret;
    if (avpkt->size < avctx->width * avctx->height * 3 / 2 + 16)
    {
        av_log(avctx, AV_LOG_ERROR, "packet too small\n");
        return AVERROR_INVALIDDATA;
    }
    if ((ret = ff_get_buffer(avctx, pic, 0)) < 0)
        return ret;
    pic->pict_type = AV_PICTURE_TYPE_I;
    pic->key_frame = 1;
    if (AV_RL32(src) != 0x01000002)
    {
        av_log_ask_for_sample(avctx, "Unknown frame header %X\n", AV_RL32(src));
        return AVERROR_PATCHWELCOME;
    }
    src += 16;
    Y1 = pic->data[0];
    Y2 = pic->data[0] + pic->linesize[0];
    U = pic->data[1];
    V = pic->data[2];
    for (h = 0; h < avctx->height; h += 2)
    {
        for (w = 0; w < avctx->width; w += 2)
        {
            AV_COPY16(Y1 + w, src);
            AV_COPY16(Y2 + w, src + 2);
            U[w >> 1] = src[4] + 0x80;
            V[w >> 1] = src[5] + 0x80;
            src += 6;
        }
        Y1 += pic->linesize[0] << 1;
        Y2 += pic->linesize[0] << 1;
        U += pic->linesize[1];
        V += pic->linesize[2];
    }
    *got_frame = 1;
    return avpkt->size;
}