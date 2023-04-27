static int libopus_decode(AVCodecContext *avc, void *data, int *got_frame_ptr, AVPacket *pkt)
{
    struct libopus_context *opus = avc->priv_data;
    AVFrame *frame = data;
    int ret, nb_samples;
    frame->nb_samples = MAX_FRAME_SIZE;
    ret = ff_get_buffer(avc, frame);
    if (ret < 0)
    {
        av_log(avc, AV_LOG_ERROR, "get_buffer() failed\n");
        return ret;
    }
    if (avc->sample_fmt == AV_SAMPLE_FMT_S16)
        nb_samples = opus_multistream_decode(opus->dec, pkt->data, pkt->size, (opus_int16 *)frame->data[0], frame->nb_samples, 0);
    else
        nb_samples = opus_multistream_decode_float(opus->dec, pkt->data, pkt->size, (float *)frame->data[0], frame->nb_samples, 0);
    if (nb_samples < 0)
    {
        av_log(avc, AV_LOG_ERROR, "Decoding error: %s\n", opus_strerror(nb_samples));
        return ff_opus_error_to_averror(nb_samples);
    }
    frame->nb_samples = nb_samples;
    *got_frame_ptr = 1;
    return pkt->size;
}