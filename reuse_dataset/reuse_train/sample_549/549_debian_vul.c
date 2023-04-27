static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    SgiState *s = avctx->priv_data;
    AVFrame *picture = data;
    AVFrame *p = &s->picture;
    unsigned int dimension, rle;
    int ret = 0;
    uint8_t *out_buf, *out_end;
    bytestream2_init(&s->g, avpkt->data, avpkt->size);
    if (bytestream2_get_bytes_left(&s->g) < SGI_HEADER_SIZE)
    {
        av_log(avctx, AV_LOG_ERROR, "buf_size too small (%d)\n", avpkt->size);
        return AVERROR_INVALIDDATA;
    }
    if (bytestream2_get_be16(&s->g) != SGI_MAGIC)
    {
        av_log(avctx, AV_LOG_ERROR, "bad magic number\n");
        return AVERROR_INVALIDDATA;
    }
    rle = bytestream2_get_byte(&s->g);
    s->bytes_per_channel = bytestream2_get_byte(&s->g);
    dimension = bytestream2_get_be16(&s->g);
    s->width = bytestream2_get_be16(&s->g);
    s->height = bytestream2_get_be16(&s->g);
    s->depth = bytestream2_get_be16(&s->g);
    if (s->bytes_per_channel != 1 && (s->bytes_per_channel != 2 || rle))
    {
        av_log(avctx, AV_LOG_ERROR, "wrong channel number\n");
        return -1;
    }
    if (dimension != 2 && dimension != 3)
    {
        av_log(avctx, AV_LOG_ERROR, "wrong dimension number\n");
        return -1;
    }
    if (s->depth == SGI_GRAYSCALE)
    {
        avctx->pix_fmt = s->bytes_per_channel == 2 ? AV_PIX_FMT_GRAY16BE : AV_PIX_FMT_GRAY8;
    }
    else if (s->depth == SGI_RGB)
    {
        avctx->pix_fmt = s->bytes_per_channel == 2 ? AV_PIX_FMT_RGB48BE : AV_PIX_FMT_RGB24;
    }
    else if (s->depth == SGI_RGBA && s->bytes_per_channel == 1)
    {
        avctx->pix_fmt = AV_PIX_FMT_RGBA;
    }
    else
    {
        av_log(avctx, AV_LOG_ERROR, "wrong picture format\n");
        return -1;
    }
    if (av_image_check_size(s->width, s->height, 0, avctx))
        return -1;
    avcodec_set_dimensions(avctx, s->width, s->height);
    if (p->data[0])
        avctx->release_buffer(avctx, p);
    p->reference = 0;
    if (ff_get_buffer(avctx, p) < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed.\n");
        return -1;
    }
    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;
    out_buf = p->data[0];
    out_end = out_buf + p->linesize[0] * s->height;
    s->linesize = p->linesize[0];
    bytestream2_seek(&s->g, SGI_HEADER_SIZE, SEEK_SET);
    if (rle)
    {
        ret = read_rle_sgi(out_end, s);
    }
    else
    {
        ret = read_uncompressed_sgi(out_buf, out_end, s);
    }
    if (ret == 0)
    {
        *picture = s->picture;
        *got_frame = 1;
        return avpkt->size;
    }
    else
    {
        return ret;
    }
}