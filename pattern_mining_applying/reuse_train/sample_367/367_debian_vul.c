static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    WNV1Context *const l = avctx->priv_data;
    const uint8_t *buf = avpkt->data;
    int buf_size = avpkt->size;
    AVFrame *const p = &l->pic;
    unsigned char *Y, *U, *V;
    int i, j, ret;
    int prev_y = 0, prev_u = 0, prev_v = 0;
    uint8_t *rbuf;
    rbuf = av_malloc(buf_size + FF_INPUT_BUFFER_PADDING_SIZE);
    if (!rbuf)
    {
        av_log(avctx, AV_LOG_ERROR, "Cannot allocate temporary buffer\n");
        return AVERROR(ENOMEM);
    }
    if (p->data[0])
        avctx->release_buffer(avctx, p);
    p->reference = 0;
    if ((ret = ff_get_buffer(avctx, p)) < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed\n");
        av_free(rbuf);
        return ret;
    }
    p->key_frame = 1;
    for (i = 8; i < buf_size; i++)
        rbuf[i] = ff_reverse[buf[i]];
    init_get_bits(&l->gb, rbuf + 8, (buf_size - 8) * 8);
    if (buf[2] >> 4 == 6)
        l->shift = 2;
    else
    {
        l->shift = 8 - (buf[2] >> 4);
        if (l->shift > 4)
        {
            av_log_ask_for_sample(avctx, "Unknown WNV1 frame header value %i\n", buf[2] >> 4);
            l->shift = 4;
        }
        if (l->shift < 1)
        {
            av_log_ask_for_sample(avctx, "Unknown WNV1 frame header value %i\n", buf[2] >> 4);
            l->shift = 1;
        }
    }
    Y = p->data[0];
    U = p->data[1];
    V = p->data[2];
    for (j = 0; j < avctx->height; j++)
    {
        for (i = 0; i < avctx->width / 2; i++)
        {
            Y[i * 2] = wnv1_get_code(l, prev_y);
            prev_u = U[i] = wnv1_get_code(l, prev_u);
            prev_y = Y[(i * 2) + 1] = wnv1_get_code(l, Y[i * 2]);
            prev_v = V[i] = wnv1_get_code(l, prev_v);
        }
        Y += p->linesize[0];
        U += p->linesize[1];
        V += p->linesize[2];
    }
    *got_frame = 1;
    *(AVFrame *)data = l->pic;
    av_free(rbuf);
    return buf_size;
}