static int dvvideo_close(AVCodecContext *c)
{
    DVVideoContext *s = c->priv_data;
    if (s->picture.data[0])
        c->release_buffer(c, &s->picture);
    return 0;
}