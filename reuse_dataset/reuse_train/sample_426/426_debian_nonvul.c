static void decode_flush(AVCodecContext *avctx)
{
    KgvContext *const c = avctx->priv_data;
    av_frame_unref(&c->prev);
}