static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    TargaContext *const s = avctx->priv_data;
    AVFrame *picture = data;
    AVFrame *const p = &s->picture;
    uint8_t *dst;
    int stride;
    int idlen, compr, y, w, h, bpp, flags, ret;
    int first_clr, colors, csize;
    bytestream2_init(&s->gb, avpkt->data, avpkt->size);
    idlen = bytestream2_get_byte(&s->gb);
    bytestream2_skip(&s->gb, 1);
    compr = bytestream2_get_byte(&s->gb);
    first_clr = bytestream2_get_le16(&s->gb);
    colors = bytestream2_get_le16(&s->gb);
    csize = bytestream2_get_byte(&s->gb);
    bytestream2_skip(&s->gb, 4);
    w = bytestream2_get_le16(&s->gb);
    h = bytestream2_get_le16(&s->gb);
    bpp = bytestream2_get_byte(&s->gb);
    flags = bytestream2_get_byte(&s->gb);
    bytestream2_skip(&s->gb, idlen);
    switch (bpp)
    {
    case 8:
        avctx->pix_fmt = ((compr & (~TGA_RLE)) == TGA_BW) ? AV_PIX_FMT_GRAY8 : AV_PIX_FMT_PAL8;
        break;
    case 15:
        avctx->pix_fmt = AV_PIX_FMT_RGB555LE;
        break;
    case 16:
        avctx->pix_fmt = AV_PIX_FMT_RGB555LE;
        break;
    case 24:
        avctx->pix_fmt = AV_PIX_FMT_BGR24;
        break;
    case 32:
        avctx->pix_fmt = AV_PIX_FMT_BGRA;
        break;
    default:
        av_log(avctx, AV_LOG_ERROR, "Bit depth %i is not supported\n", bpp);
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
    if (flags & 0x20)
    {
        dst = p->data[0];
        stride = p->linesize[0];
    }
    else
    {
        dst = p->data[0] + p->linesize[0] * (h - 1);
        stride = -p->linesize[0];
    }
    if (colors)
    {
        int pal_size, pal_sample_size;
        if ((colors + first_clr) > 256)
        {
            av_log(avctx, AV_LOG_ERROR, "Incorrect palette: %i colors with offset %i\n", colors, first_clr);
            return AVERROR_INVALIDDATA;
        }
        switch (csize)
        {
        case 24:
            pal_sample_size = 3;
            break;
        case 16:
        case 15:
            pal_sample_size = 2;
            break;
        default:
            av_log(avctx, AV_LOG_ERROR, "Palette entry size %i bits is not supported\n", csize);
            return AVERROR_INVALIDDATA;
        }
        pal_size = colors * pal_sample_size;
        if (avctx->pix_fmt != AV_PIX_FMT_PAL8)
            bytestream2_skip(&s->gb, pal_size);
        else
        {
            int t;
            uint32_t *pal = ((uint32_t *)p->data[1]) + first_clr;
            if (bytestream2_get_bytes_left(&s->gb) < pal_size)
            {
                av_log(avctx, AV_LOG_ERROR, "Not enough data to read palette\n");
                return AVERROR_INVALIDDATA;
            }
            switch (pal_sample_size)
            {
            case 3:
                for (t = 0; t < colors; t++)
                    *pal++ = bytestream2_get_le24u(&s->gb);
                break;
            case 2:
                for (t = 0; t < colors; t++)
                {
                    uint32_t v = bytestream2_get_le16u(&s->gb);
                    v = ((v & 0x7C00) << 9) | ((v & 0x03E0) << 6) | ((v & 0x001F) << 3);
                    v |= (v & 0xE0E0E0U) >> 5;
                    *pal++ = v;
                }
                break;
            }
            p->palette_has_changed = 1;
        }
    }
    if ((compr & (~TGA_RLE)) == TGA_NODATA)
    {
        memset(p->data[0], 0, p->linesize[0] * h);
    }
    else
    {
        if (compr & TGA_RLE)
        {
            int res = targa_decode_rle(avctx, s, dst, w, h, stride, bpp);
            if (res < 0)
                return res;
        }
        else
        {
            size_t img_size = w * ((bpp + 1) >> 3);
            if (bytestream2_get_bytes_left(&s->gb) < img_size * h)
            {
                av_log(avctx, AV_LOG_ERROR, "Not enough data available for image\n");
                return AVERROR_INVALIDDATA;
            }
            for (y = 0; y < h; y++)
            {
                bytestream2_get_bufferu(&s->gb, dst, img_size);
                dst += stride;
            }
        }
    }
    *picture = s->picture;
    *got_frame = 1;
    return avpkt->size;
}