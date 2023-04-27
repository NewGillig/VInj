static int decode_picture_header(ProresContext *ctx, const uint8_t *buf, const int data_size, AVCodecContext *avctx)
{
    int i, hdr_size, pic_data_size, num_slices;
    int slice_width_factor, slice_height_factor;
    int remainder, num_x_slices;
    const uint8_t *data_ptr, *index_ptr;
    hdr_size = data_size > 0 ? buf[0] >> 3 : 0;
    if (hdr_size < 8 || hdr_size > data_size)
    {
        av_log(avctx, AV_LOG_ERROR, "picture header too small\n");
        return AVERROR_INVALIDDATA;
    }
    pic_data_size = AV_RB32(buf + 1);
    if (pic_data_size > data_size)
    {
        av_log(avctx, AV_LOG_ERROR, "picture data too small\n");
        return AVERROR_INVALIDDATA;
    }
    slice_width_factor = buf[7] >> 4;
    slice_height_factor = buf[7] & 0xF;
    if (slice_width_factor > 3 || slice_height_factor)
    {
        av_log(avctx, AV_LOG_ERROR, "unsupported slice dimension: %d x %d\n", 1 << slice_width_factor, 1 << slice_height_factor);
        return AVERROR_INVALIDDATA;
    }
    ctx->slice_width_factor = slice_width_factor;
    ctx->slice_height_factor = slice_height_factor;
    ctx->num_x_mbs = (avctx->width + 15) >> 4;
    ctx->num_y_mbs = (avctx->height + (1 << (4 + ctx->picture.interlaced_frame)) - 1) >> (4 + ctx->picture.interlaced_frame);
    remainder = ctx->num_x_mbs & ((1 << slice_width_factor) - 1);
    num_x_slices = (ctx->num_x_mbs >> slice_width_factor) + (remainder & 1) + ((remainder >> 1) & 1) + ((remainder >> 2) & 1);
    num_slices = num_x_slices * ctx->num_y_mbs;
    if (num_slices != AV_RB16(buf + 5))
    {
        av_log(avctx, AV_LOG_ERROR, "invalid number of slices\n");
        return AVERROR_INVALIDDATA;
    }
    if (ctx->total_slices != num_slices)
    {
        av_freep(&ctx->slice_data);
        ctx->slice_data = av_malloc((num_slices + 1) * sizeof(ctx->slice_data[0]));
        if (!ctx->slice_data)
            return AVERROR(ENOMEM);
        ctx->total_slices = num_slices;
    }
    if (hdr_size + num_slices * 2 > data_size)
    {
        av_log(avctx, AV_LOG_ERROR, "slice table too small\n");
        return AVERROR_INVALIDDATA;
    }
    index_ptr = buf + hdr_size;
    data_ptr = index_ptr + num_slices * 2;
    for (i = 0; i < num_slices; i++)
    {
        ctx->slice_data[i].index = data_ptr;
        ctx->slice_data[i].prev_slice_sf = 0;
        data_ptr += AV_RB16(index_ptr + i * 2);
    }
    ctx->slice_data[i].index = data_ptr;
    ctx->slice_data[i].prev_slice_sf = 0;
    if (data_ptr > buf + data_size)
    {
        av_log(avctx, AV_LOG_ERROR, "out of slice data\n");
        return -1;
    }
    return pic_data_size;
}