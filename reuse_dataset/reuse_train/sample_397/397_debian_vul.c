static int mss1_decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size = avpkt->size;
    MSS1Context *ctx = avctx->priv_data;
    MSS12Context *c = &ctx->ctx;
    GetBitContext gb;
    ArithCoder acoder;
    int pal_changed = 0;
    int ret;
    init_get_bits(&gb, buf, buf_size * 8);
    arith_init(&acoder, &gb);
    ctx->pic.reference = 3;
    ctx->pic.buffer_hints = FF_BUFFER_HINTS_VALID | FF_BUFFER_HINTS_READABLE | FF_BUFFER_HINTS_PRESERVE | FF_BUFFER_HINTS_REUSABLE;
    if ((ret = avctx->reget_buffer(avctx, &ctx->pic)) < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "reget_buffer() failed\n");
        return ret;
    }
    c->pal_pic = ctx->pic.data[0] + ctx->pic.linesize[0] * (avctx->height - 1);
    c->pal_stride = -ctx->pic.linesize[0];
    c->keyframe = !arith_get_bit(&acoder);
    if (c->keyframe)
    {
        c->corrupted = 0;
        ff_mss12_slicecontext_reset(&ctx->sc);
        pal_changed = decode_pal(c, &acoder);
        ctx->pic.key_frame = 1;
        ctx->pic.pict_type = AV_PICTURE_TYPE_I;
    }
    else
    {
        if (c->corrupted)
            return AVERROR_INVALIDDATA;
        ctx->pic.key_frame = 0;
        ctx->pic.pict_type = AV_PICTURE_TYPE_P;
    }
    c->corrupted = ff_mss12_decode_rect(&ctx->sc, &acoder, 0, 0, avctx->width, avctx->height);
    if (c->corrupted)
        return AVERROR_INVALIDDATA;
    memcpy(ctx->pic.data[1], c->pal, AVPALETTE_SIZE);
    ctx->pic.palette_has_changed = pal_changed;
    *got_frame = 1;
    *(AVFrame *)data = ctx->pic;
    return buf_size;
}