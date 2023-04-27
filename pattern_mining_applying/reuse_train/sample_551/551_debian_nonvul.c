static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size = avpkt->size;
    C93DecoderContext *const c93 = avctx->priv_data;
    AVFrame *const newpic = &c93->pictures[c93->currentpic];
    AVFrame *const oldpic = &c93->pictures[c93->currentpic ^ 1];
    GetByteContext gb;
    uint8_t *out;
    int stride, ret, i, x, y, b, bt = 0;
    c93->currentpic ^= 1;
    if ((ret = ff_reget_buffer(avctx, newpic)) < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "reget_buffer() failed\n");
        return ret;
    }
    stride = newpic->linesize[0];
    bytestream2_init(&gb, buf, buf_size);
    b = bytestream2_get_byte(&gb);
    if (b & C93_FIRST_FRAME)
    {
        newpic->pict_type = AV_PICTURE_TYPE_I;
        newpic->key_frame = 1;
    }
    else
    {
        newpic->pict_type = AV_PICTURE_TYPE_P;
        newpic->key_frame = 0;
    }
    for (y = 0; y < HEIGHT; y += 8)
    {
        out = newpic->data[0] + y * stride;
        for (x = 0; x < WIDTH; x += 8)
        {
            uint8_t *copy_from = oldpic->data[0];
            unsigned int offset, j;
            uint8_t cols[4], grps[4];
            C93BlockType block_type;
            if (!bt)
                bt = bytestream2_get_byte(&gb);
            block_type = bt & 0x0F;
            switch (block_type)
            {
            case C93_8X8_FROM_PREV:
                offset = bytestream2_get_le16(&gb);
                if ((ret = copy_block(avctx, out, copy_from, offset, 8, stride)) < 0)
                    return ret;
                break;
            case C93_4X4_FROM_CURR:
                copy_from = newpic->data[0];
            case C93_4X4_FROM_PREV:
                for (j = 0; j < 8; j += 4)
                {
                    for (i = 0; i < 8; i += 4)
                    {
                        offset = bytestream2_get_le16(&gb);
                        if ((ret = copy_block(avctx, &out[j * stride + i], copy_from, offset, 4, stride)) < 0)
                            return ret;
                    }
                }
                break;
            case C93_8X8_2COLOR:
                bytestream2_get_buffer(&gb, cols, 2);
                for (i = 0; i < 8; i++)
                {
                    draw_n_color(out + i * stride, stride, 8, 1, 1, cols, NULL, bytestream2_get_byte(&gb));
                }
                break;
            case C93_4X4_2COLOR:
            case C93_4X4_4COLOR:
            case C93_4X4_4COLOR_GRP:
                for (j = 0; j < 8; j += 4)
                {
                    for (i = 0; i < 8; i += 4)
                    {
                        if (block_type == C93_4X4_2COLOR)
                        {
                            bytestream2_get_buffer(&gb, cols, 2);
                            draw_n_color(out + i + j * stride, stride, 4, 4, 1, cols, NULL, bytestream2_get_le16(&gb));
                        }
                        else if (block_type == C93_4X4_4COLOR)
                        {
                            bytestream2_get_buffer(&gb, cols, 4);
                            draw_n_color(out + i + j * stride, stride, 4, 4, 2, cols, NULL, bytestream2_get_le32(&gb));
                        }
                        else
                        {
                            bytestream2_get_buffer(&gb, grps, 4);
                            draw_n_color(out + i + j * stride, stride, 4, 4, 1, cols, grps, bytestream2_get_le16(&gb));
                        }
                    }
                }
                break;
            case C93_NOOP:
                break;
            case C93_8X8_INTRA:
                for (j = 0; j < 8; j++)
                    bytestream2_get_buffer(&gb, out + j * stride, 8);
                break;
            default:
                av_log(avctx, AV_LOG_ERROR, "unexpected type %x at %dx%d\n", block_type, x, y);
                return AVERROR_INVALIDDATA;
            }
            bt >>= 4;
            out += 8;
        }
    }
    if (b & C93_HAS_PALETTE)
    {
        uint32_t *palette = (uint32_t *)newpic->data[1];
        for (i = 0; i < 256; i++)
        {
            palette[i] = bytestream2_get_be24(&gb);
        }
        newpic->palette_has_changed = 1;
    }
    else
    {
        if (oldpic->data[1])
            memcpy(newpic->data[1], oldpic->data[1], 256 * 4);
    }
    if ((ret = av_frame_ref(data, newpic)) < 0)
        return ret;
    *got_frame = 1;
    return buf_size;
}