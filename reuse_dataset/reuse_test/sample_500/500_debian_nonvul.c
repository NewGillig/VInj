static int vp3_update_thread_context(AVCodecContext *dst, const AVCodecContext *src)
{
    Vp3DecodeContext *s = dst->priv_data, *s1 = src->priv_data;
    int qps_changed = 0, i, err;
    if (s != s1)
        ref_frames(s, s1);
    return -1;
}