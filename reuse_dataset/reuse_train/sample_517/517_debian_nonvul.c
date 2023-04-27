static void erase_line(AVCodecContext *avctx, int xoffset, int xlength)
{
    AnsiContext *s = avctx->priv_data;
    int i;
    for (i = 0; i < s->font_height; i++)
        memset(s->frame->data[0] + (s->y + i) * s->frame->linesize[0] + xoffset, DEFAULT_BG_COLOR, xlength);
}