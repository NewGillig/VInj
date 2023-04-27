static int dvvideo_decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    uint8_t *buf = avpkt->data;
    int buf_size = avpkt->size;
    DVVideoContext *s = avctx->priv_data;
    const uint8_t *vsc_pack;
    int apt, is16_9;
    s->sys = avpriv_dv_frame_profile(s->sys, buf, buf_size);
    if (!s->sys || buf_size < s->sys->frame_size || ff_dv_init_dynamic_tables(s->sys))
    {
        av_log(avctx, AV_LOG_ERROR, "could not find dv frame profile\n");
        return -1;
    }
    if (s->picture.data[0])
        avctx->release_buffer(avctx, &s->picture);
    s->picture.reference = 0;
    s->picture.key_frame = 1;
    s->picture.pict_type = AV_PICTURE_TYPE_I;
    avctx->pix_fmt = s->sys->pix_fmt;
    avctx->time_base = s->sys->time_base;
    avcodec_set_dimensions(avctx, s->sys->width, s->sys->height);
    if (ff_get_buffer(avctx, &s->picture) < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed\n");
        return -1;
    }
    s->picture.interlaced_frame = 1;
    s->picture.top_field_first = 0;
    s->buf = buf;
    avctx->execute(avctx, dv_decode_video_segment, s->sys->work_chunks, NULL, dv_work_pool_size(s->sys), sizeof(DVwork_chunk));
    emms_c();
    *got_frame = 1;
    *(AVFrame *)data = s->picture;
    vsc_pack = buf + 80 * 5 + 48 + 5;
    if (*vsc_pack == dv_video_control)
    {
        apt = buf[4] & 0x07;
        is16_9 = (vsc_pack && ((vsc_pack[2] & 0x07) == 0x02 || (!apt && (vsc_pack[2] & 0x07) == 0x07)));
        avctx->sample_aspect_ratio = s->sys->sar[is16_9];
    }
    return s->sys->frame_size;
}