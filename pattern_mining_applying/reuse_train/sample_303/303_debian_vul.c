static int bethsoftvid_decode_frame(AVCodecContext *avctx, void *data, int *got_frame, AVPacket *avpkt)
{
    BethsoftvidContext *vid = avctx->priv_data;
    char block_type;
    uint8_t *dst;
    uint8_t *frame_end;
    int remaining = avctx->width;
    int wrap_to_next_line;
    int code, ret;
    int yoffset;
    if ((ret = avctx->reget_buffer(avctx, &vid->frame)) < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "reget_buffer() failed\n");
        return ret;
    }
    wrap_to_next_line = vid->frame.linesize[0] - avctx->width;
    if (avpkt->side_data_elems > 0 && avpkt->side_data[0].type == AV_PKT_DATA_PALETTE)
    {
        bytestream2_init(&vid->g, avpkt->side_data[0].data, avpkt->side_data[0].size);
        if ((ret = set_palette(vid)) < 0)
            return ret;
    }
    bytestream2_init(&vid->g, avpkt->data, avpkt->size);
    dst = vid->frame.data[0];
    frame_end = vid->frame.data[0] + vid->frame.linesize[0] * avctx->height;
    switch (block_type = bytestream2_get_byte(&vid->g))
    {
    case PALETTE_BLOCK:
    {
        *got_frame = 0;
        if ((ret = set_palette(vid)) < 0)
        {
            av_log(avctx, AV_LOG_ERROR, "error reading palette\n");
            return ret;
        }
        return bytestream2_tell(&vid->g);
    }
    case VIDEO_YOFF_P_FRAME:
        yoffset = bytestream2_get_le16(&vid->g);
        if (yoffset >= avctx->height)
            return AVERROR_INVALIDDATA;
        dst += vid->frame.linesize[0] * yoffset;
    }
    while ((code = bytestream2_get_byte(&vid->g)))
    {
        int length = code & 0x7f;
        while (length > remaining)
        {
            if (code < 0x80)
                bytestream2_get_buffer(&vid->g, dst, remaining);
            else if (block_type == VIDEO_I_FRAME)
                memset(dst, bytestream2_peek_byte(&vid->g), remaining);
            length -= remaining;
            dst += remaining + wrap_to_next_line;
            remaining = avctx->width;
            if (dst == frame_end)
                goto end;
        }
        if (code < 0x80)
            bytestream2_get_buffer(&vid->g, dst, length);
        else if (block_type == VIDEO_I_FRAME)
            memset(dst, bytestream2_get_byte(&vid->g), length);
        remaining -= length;
        dst += length;
    }
end:
    *got_frame = 1;
    *(AVFrame *)data = vid->frame;
    return avpkt->size;
}