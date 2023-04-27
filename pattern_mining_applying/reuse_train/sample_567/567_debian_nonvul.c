static int dvvideo_close(AVCodecContext *c)
{
    DVVideoContext *s = c->priv_data;
    av_frame_unref(&s->picture);
    return 0;
}