static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    const uint8_t *buf_end = avpkt->data + avpkt->size;
    int buf_size = avpkt->size;
    DPXContext *const s = avctx->priv_data;
    AVFrame *picture = data;
    AVFrame *const p = &s->picture;
    uint8_t *ptr;
    unsigned int offset;
    int magic_num, endian;
    int x, y, ret;
    int w, h, stride, bits_per_color, descriptor, elements, target_packet_size, source_packet_size;
    unsigned int rgbBuffer;
    if (avpkt->size <= 1634)
    {
        av_log(avctx, AV_LOG_ERROR, "Packet too small for DPX header\n");
        return AVERROR_INVALIDDATA;
    }
    magic_num = AV_RB32(buf);
    buf += 4;
    if (magic_num == AV_RL32("SDPX"))
    {
        endian = 0;
    }
    else if (magic_num == AV_RB32("SDPX"))
    {
        endian = 1;
    }
    else
    {
        av_log(avctx, AV_LOG_ERROR, "DPX marker not found\n");
        return AVERROR_INVALIDDATA;
    }
    offset = read32(&buf, endian);
    if (avpkt->size <= offset)
    {
        av_log(avctx, AV_LOG_ERROR, "Invalid data start offset\n");
        return AVERROR_INVALIDDATA;
    }
    buf = avpkt->data + 0x304;
    w = read32(&buf, endian);
    h = read32(&buf, endian);
    buf += 20;
    descriptor = buf[0];
    buf += 3;
    avctx->bits_per_raw_sample = bits_per_color = buf[0];
    buf += 825;
    avctx->sample_aspect_ratio.num = read32(&buf, endian);
    avctx->sample_aspect_ratio.den = read32(&buf, endian);
    switch (descriptor)
    {
    case 51:
        elements = 4;
        break;
    case 50:
        elements = 3;
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "Unsupported descriptor %d\n", descriptor);
        return AVERROR_INVALIDDATA;
    }
    switch (bits_per_color)
    {
    case 8:
        if (elements == 4)
        {
            avctx->pix_fmt = AV_PIX_FMT_RGBA;
        }
        else
        {
            avctx->pix_fmt = AV_PIX_FMT_RGB24;
        }
        source_packet_size = elements;
        target_packet_size = elements;
        break;
    case 10:
        avctx->pix_fmt = AV_PIX_FMT_RGB48;
        target_packet_size = 6;
        source_packet_size = 4;
        break;
    case 12:
    case 16:
        if (endian)
        {
            avctx->pix_fmt = AV_PIX_FMT_RGB48BE;
        }
        else
        {
            avctx->pix_fmt = AV_PIX_FMT_RGB48LE;
        }
        target_packet_size = 6;
        source_packet_size = elements * 2;
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "Unsupported color depth : %d\n", bits_per_color);
        return AVERROR_INVALIDDATA;
    }
    if (s->picture.data[0])
        avctx->release_buffer(avctx, &s->picture);
    if ((ret = av_image_check_size(w, h, 0, avctx)) < 0)
        return ret;
    if (w != avctx->width || h != avctx->height)
        avcodec_set_dimensions(avctx, w, h);
    if ((ret = ff_get_buffer(avctx, p)) < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed\n");
        return ret;
    }
    buf = avpkt->data + offset;
    ptr = p->data[0];
    stride = p->linesize[0];
    if (source_packet_size * avctx->width * avctx->height > buf_end - buf)
    {
        av_log(avctx, AV_LOG_ERROR, "Overread buffer. Invalid header?\n");
        return AVERROR_INVALIDDATA;
    }
    switch (bits_per_color)
    {
    case 10:
        for (x = 0; x < avctx->height; x++)
        {
            uint16_t *dst = (uint16_t *)ptr;
            for (y = 0; y < avctx->width; y++)
            {
                rgbBuffer = read32(&buf, endian);
                *dst++ = make_16bit(rgbBuffer >> 16);
                *dst++ = make_16bit(rgbBuffer >> 6);
                *dst++ = make_16bit(rgbBuffer << 4);
            }
            ptr += stride;
        }
        break;
    case 8:
    case 12:
    case 16:
        if (source_packet_size == target_packet_size)
        {
            for (x = 0; x < avctx->height; x++)
            {
                memcpy(ptr, buf, target_packet_size * avctx->width);
                ptr += stride;
                buf += source_packet_size * avctx->width;
            }
        }
        else
        {
            for (x = 0; x < avctx->height; x++)
            {
                uint8_t *dst = ptr;
                for (y = 0; y < avctx->width; y++)
                {
                    memcpy(dst, buf, target_packet_size);
                    dst += target_packet_size;
                    buf += source_packet_size;
                }
                ptr += stride;
            }
        }
        break;
    }
    *picture = s->picture;
    *got_frame = 1;
    return buf_size;
}