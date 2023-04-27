static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    int h, w, ret;
    AVFrame *pic = avctx->coded_frame;
    const uint32_t *src = (const uint32_t *)avpkt->data;
    int aligned_width = FFALIGN(avctx->width, 64);
    uint8_t *dst_line;
    if (pic->data[0])
        avctx->release_buffer(avctx, pic);
    if (avpkt->size < 4 * aligned_width * avctx->height)
    {
        av_log(avctx, AV_LOG_ERROR, "packet too small\n");
        return AVERROR_INVALIDDATA;
    }
    pic->reference = 0;
    if ((ret = ff_get_buffer(avctx, pic)) < 0)
        return ret;
    pic->pict_type = AV_PICTURE_TYPE_I;
    pic->key_frame = 1;
    dst_line = pic->data[0];
    for (h = 0; h < avctx->height; h++)
    {
        uint16_t *dst = (uint16_t *)dst_line;
        for (w = 0; w < avctx->width; w++)
        {
            uint32_t pixel = av_be2ne32(*src++);
            uint16_t r, g, b;
            if (avctx->codec_id == AV_CODEC_ID_R210)
            {
                b = pixel << 6;
                g = (pixel >> 4) & 0xffc0;
                r = (pixel >> 14) & 0xffc0;
            }
            else
            {
                b = pixel << 4;
                g = (pixel >> 6) & 0xffc0;
                r = (pixel >> 16) & 0xffc0;
            }
            *dst++ = r | (r >> 10);
            *dst++ = g | (g >> 10);
            *dst++ = b | (b >> 10);
        }
        src += aligned_width - avctx->width;
        dst_line += pic->linesize[0];
    }
    *got_frame = 1;
    *(AVFrame *)data = *avctx->coded_frame;
    return avpkt->size;
}