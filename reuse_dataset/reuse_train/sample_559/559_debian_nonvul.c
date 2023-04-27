static int qtrle_decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    QtrleContext *s = avctx->priv_data;
    int header, start_line;
    int height, row_ptr;
    int has_palette = 0;
    int ret;
    bytestream2_init(&s->g, avpkt->data, avpkt->size);
    if ((ret = ff_reget_buffer(avctx, &s->frame)) < 0)
    {
        av_log(s->avctx, AV_LOG_ERROR, "reget_buffer() failed\n");
        return ret;
    }
    if (avpkt->size < 8)
        goto done;
    bytestream2_seek(&s->g, 4, SEEK_SET);
    header = bytestream2_get_be16(&s->g);
    if (header & 0x0008)
    {
        if (avpkt->size < 14)
            goto done;
        start_line = bytestream2_get_be16(&s->g);
        bytestream2_skip(&s->g, 2);
        height = bytestream2_get_be16(&s->g);
        bytestream2_skip(&s->g, 2);
    }
    else
    {
        start_line = 0;
        height = s->avctx->height;
    }
    row_ptr = s->frame.linesize[0] * start_line;
    switch (avctx->bits_per_coded_sample)
    {
    case 1:
    case 33:
        qtrle_decode_1bpp(s, row_ptr, height);
        break;
    case 2:
    case 34:
        qtrle_decode_2n4bpp(s, row_ptr, height, 2);
        has_palette = 1;
        break;
    case 4:
    case 36:
        qtrle_decode_2n4bpp(s, row_ptr, height, 4);
        has_palette = 1;
        break;
    case 8:
    case 40:
        qtrle_decode_8bpp(s, row_ptr, height);
        has_palette = 1;
        break;
    case 16:
        qtrle_decode_16bpp(s, row_ptr, height);
        break;
    case 24:
        qtrle_decode_24bpp(s, row_ptr, height);
        break;
    case 32:
        qtrle_decode_32bpp(s, row_ptr, height);
        break;
    default:
        av_log(s->avctx, AV_LOG_ERROR, "Unsupported colorspace: %d bits/sample?\n", avctx->bits_per_coded_sample);
        break;
    }
    if (has_palette)
    {
        const uint8_t *pal = av_packet_get_side_data(avpkt, AV_PKT_DATA_PALETTE, NULL);
        if (pal)
        {
            s->frame.palette_has_changed = 1;
            memcpy(s->pal, pal, AVPALETTE_SIZE);
        }
        memcpy(s->frame.data[1], s->pal, AVPALETTE_SIZE);
    }
done:
    if ((ret = av_frame_ref(data, &s->frame)) < 0)
        return ret;
    *got_frame = 1;
    return avpkt->size;
}