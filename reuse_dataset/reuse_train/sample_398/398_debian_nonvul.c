static int flic_decode_frame_15_16BPP(AVCodecContext *avctx, void *data, int *got_frame, const uint8_t *buf, int buf_size)
{
    FlicDecodeContext *s = avctx->priv_data;
    GetByteContext g2;
    int pixel_ptr;
    unsigned char palette_idx1;
    unsigned int frame_size;
    int num_chunks;
    unsigned int chunk_size;
    int chunk_type;
    int i, j, ret;
    int lines;
    int compressed_lines;
    signed short line_packets;
    int y_ptr;
    int byte_run;
    int pixel_skip;
    int pixel_countdown;
    unsigned char *pixels;
    int pixel;
    unsigned int pixel_limit;
    bytestream2_init(&g2, buf, buf_size);
    if ((ret = ff_reget_buffer(avctx, &s->frame)) < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "reget_buffer() failed\n");
        return ret;
    }
    pixels = s->frame.data[0];
    pixel_limit = s->avctx->height * s->frame.linesize[0];
    frame_size = bytestream2_get_le32(&g2);
    bytestream2_skip(&g2, 2);
    num_chunks = bytestream2_get_le16(&g2);
    bytestream2_skip(&g2, 8);
    frame_size -= 16;
    while ((frame_size > 0) && (num_chunks > 0))
    {
        chunk_size = bytestream2_get_le32(&g2);
        chunk_type = bytestream2_get_le16(&g2);
        switch (chunk_type)
        {
        case FLI_256_COLOR:
        case FLI_COLOR:
            av_dlog(avctx, "Unexpected Palette chunk %d in non-palettized FLC\n", chunk_type);
            bytestream2_skip(&g2, chunk_size - 6);
            break;
        case FLI_DELTA:
        case FLI_DTA_LC:
            y_ptr = 0;
            compressed_lines = bytestream2_get_le16(&g2);
            while (compressed_lines > 0)
            {
                line_packets = bytestream2_get_le16(&g2);
                if (line_packets < 0)
                {
                    line_packets = -line_packets;
                    y_ptr += line_packets * s->frame.linesize[0];
                }
                else
                {
                    compressed_lines--;
                    pixel_ptr = y_ptr;
                    CHECK_PIXEL_PTR(0);
                    pixel_countdown = s->avctx->width;
                    for (i = 0; i < line_packets; i++)
                    {
                        pixel_skip = bytestream2_get_byte(&g2);
                        pixel_ptr += (pixel_skip * 2);
                        pixel_countdown -= pixel_skip;
                        byte_run = sign_extend(bytestream2_get_byte(&g2), 8);
                        if (byte_run < 0)
                        {
                            byte_run = -byte_run;
                            pixel = bytestream2_get_le16(&g2);
                            CHECK_PIXEL_PTR(2 * byte_run);
                            for (j = 0; j < byte_run; j++, pixel_countdown -= 2)
                            {
                                *((signed short *)(&pixels[pixel_ptr])) = pixel;
                                pixel_ptr += 2;
                            }
                        }
                        else
                        {
                            CHECK_PIXEL_PTR(2 * byte_run);
                            for (j = 0; j < byte_run; j++, pixel_countdown--)
                            {
                                *((signed short *)(&pixels[pixel_ptr])) = bytestream2_get_le16(&g2);
                                pixel_ptr += 2;
                            }
                        }
                    }
                    y_ptr += s->frame.linesize[0];
                }
            }
            break;
        case FLI_LC:
            av_log(avctx, AV_LOG_ERROR, "Unexpected FLI_LC chunk in non-paletised FLC\n");
            bytestream2_skip(&g2, chunk_size - 6);
            break;
        case FLI_BLACK:
            memset(pixels, 0x0000, s->frame.linesize[0] * s->avctx->height);
            break;
        case FLI_BRUN:
            y_ptr = 0;
            for (lines = 0; lines < s->avctx->height; lines++)
            {
                pixel_ptr = y_ptr;
                bytestream2_skip(&g2, 1);
                pixel_countdown = (s->avctx->width * 2);
                while (pixel_countdown > 0)
                {
                    byte_run = sign_extend(bytestream2_get_byte(&g2), 8);
                    if (byte_run > 0)
                    {
                        palette_idx1 = bytestream2_get_byte(&g2);
                        CHECK_PIXEL_PTR(byte_run);
                        for (j = 0; j < byte_run; j++)
                        {
                            pixels[pixel_ptr++] = palette_idx1;
                            pixel_countdown--;
                            if (pixel_countdown < 0)
                                av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d) (linea%d)\n", pixel_countdown, lines);
                        }
                    }
                    else
                    {
                        byte_run = -byte_run;
                        CHECK_PIXEL_PTR(byte_run);
                        for (j = 0; j < byte_run; j++)
                        {
                            palette_idx1 = bytestream2_get_byte(&g2);
                            pixels[pixel_ptr++] = palette_idx1;
                            pixel_countdown--;
                            if (pixel_countdown < 0)
                                av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d) at line %d\n", pixel_countdown, lines);
                        }
                    }
                }
                pixel_countdown = s->avctx->width;
                while (pixel_countdown > 0)
                {
                    *((signed short *)(&pixels[pixel_ptr])) = AV_RL16(&buf[pixel_ptr]);
                    pixel_ptr += 2;
                }
            }
            break;
        case FLI_DTA_BRUN:
            y_ptr = 0;
            for (lines = 0; lines < s->avctx->height; lines++)
            {
                pixel_ptr = y_ptr;
                bytestream2_skip(&g2, 1);
                pixel_countdown = s->avctx->width;
                while (pixel_countdown > 0)
                {
                    byte_run = sign_extend(bytestream2_get_byte(&g2), 8);
                    if (byte_run > 0)
                    {
                        pixel = bytestream2_get_le16(&g2);
                        CHECK_PIXEL_PTR(2 * byte_run);
                        for (j = 0; j < byte_run; j++)
                        {
                            *((signed short *)(&pixels[pixel_ptr])) = pixel;
                            pixel_ptr += 2;
                            pixel_countdown--;
                            if (pixel_countdown < 0)
                                av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d)\n", pixel_countdown);
                        }
                    }
                    else
                    {
                        byte_run = -byte_run;
                        CHECK_PIXEL_PTR(2 * byte_run);
                        for (j = 0; j < byte_run; j++)
                        {
                            *((signed short *)(&pixels[pixel_ptr])) = bytestream2_get_le16(&g2);
                            pixel_ptr += 2;
                            pixel_countdown--;
                            if (pixel_countdown < 0)
                                av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d)\n", pixel_countdown);
                        }
                    }
                }
                y_ptr += s->frame.linesize[0];
            }
            break;
        case FLI_COPY:
        case FLI_DTA_COPY:
            if (chunk_size - 6 > (unsigned int)(s->avctx->width * s->avctx->height) * 2)
            {
                av_log(avctx, AV_LOG_ERROR, "In chunk FLI_COPY : source data (%d bytes) " \ "bigger than image, skipping chunk\n", chunk_size - 6);
                bytestream2_skip(&g2, chunk_size - 6);
            }
            else
            {
                for (y_ptr = 0; y_ptr < s->frame.linesize[0] * s->avctx->height; y_ptr += s->frame.linesize[0])
                {
                    pixel_countdown = s->avctx->width;
                    pixel_ptr = 0;
                    while (pixel_countdown > 0)
                    {
                        *((signed short *)(&pixels[y_ptr + pixel_ptr])) = bytestream2_get_le16(&g2);
                        pixel_ptr += 2;
                        pixel_countdown--;
                    }
                }
            }
            break;
        case FLI_MINI:
            bytestream2_skip(&g2, chunk_size - 6);
            break;
        default:
            av_log(avctx, AV_LOG_ERROR, "Unrecognized chunk type: %d\n", chunk_type);
            break;
        }
        frame_size -= chunk_size;
        num_chunks--;
    }
    if ((bytestream2_get_bytes_left(&g2) != 0) && (bytestream2_get_bytes_left(&g2) != 1))
        av_log(avctx, AV_LOG_ERROR, "Processed FLI chunk where chunk size = %d " \ "and final chunk ptr = %d\n", buf_size, bytestream2_tell(&g2));
    if ((ret = av_frame_ref(data, &s->frame)) < 0)
        return ret;
    *got_frame = 1;
    return buf_size;
}