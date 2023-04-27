static void draw_char(AVCodecContext *avctx, int c)
{
    AnsiContext *s = avctx->priv_data;
    int fg = s->fg;
    int bg = s->bg;
    if ((s->attributes & ATTR_BOLD))
        fg += 8;
    if ((s->attributes & ATTR_BLINK))
        bg += 8;
    if ((s->attributes & ATTR_REVERSE))
        FFSWAP(int, fg, bg);
    if ((s->attributes & ATTR_CONCEALED))
        fg = bg;
    ff_draw_pc_font(s->frame.data[0] + s->y * s->frame.linesize[0] + s->x, s->frame.linesize[0], s->font, s->font_height, c, fg, bg);
    s->x += FONT_WIDTH;
    if (s->x >= avctx->width)
    {
        s->x = 0;
        hscroll(avctx);
    }
}