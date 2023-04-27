static void decode_flush(AVCodecContext *avctx)
{
    KgvContext *const c = avctx->priv_data;
    if (c->prev.data[0])
        avctx->release_buffer(avctx, &c->prev);
}