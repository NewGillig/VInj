static void erase_screen(AVCodecContext *avctx)
{
    AnsiContext *s = avctx->priv_data;
    int i;
    for (i = 0; i < avctx->height; i++)
        memset(s->frame->data[0] + i * s->frame->linesize[0], DEFAULT_BG_COLOR, avctx->width);
    s->x = s->y = 0;
}