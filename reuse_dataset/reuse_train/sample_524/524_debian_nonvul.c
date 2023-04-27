static int truemotion1_decode_header(TrueMotion1Context *s)
{
    int i, ret;
    int width_shift = 0;
    int new_pix_fmt;
    struct frame_header header;
    uint8_t header_buffer[128] = {0};
    const uint8_t *sel_vector_table;
    header.header_size = ((s->buf[0] >> 5) | (s->buf[0] << 3)) & 0x7f;
    if (s->buf[0] < 0x10)
    {
        av_log(s->avctx, AV_LOG_ERROR, "invalid header size (%d)\n", s->buf[0]);
        return AVERROR_INVALIDDATA;
    }
    for (i = 1; i < header.header_size; i++)
        header_buffer[i - 1] = s->buf[i] ^ s->buf[i + 1];
    header.compression = header_buffer[0];
    header.deltaset = header_buffer[1];
    header.vectable = header_buffer[2];
    header.ysize = AV_RL16(&header_buffer[3]);
    header.xsize = AV_RL16(&header_buffer[5]);
    header.checksum = AV_RL16(&header_buffer[7]);
    header.version = header_buffer[9];
    header.header_type = header_buffer[10];
    header.flags = header_buffer[11];
    header.control = header_buffer[12];
    if (header.version >= 2)
    {
        if (header.header_type > 3)
        {
            av_log(s->avctx, AV_LOG_ERROR, "invalid header type (%d)\n", header.header_type);
            return AVERROR_INVALIDDATA;
        }
        else if ((header.header_type == 2) || (header.header_type == 3))
        {
            s->flags = header.flags;
            if (!(s->flags & FLAG_INTERFRAME))
                s->flags |= FLAG_KEYFRAME;
        }
        else
            s->flags = FLAG_KEYFRAME;
    }
    else
        s->flags = FLAG_KEYFRAME;
    if (s->flags & FLAG_SPRITE)
    {
        av_log_ask_for_sample(s->avctx, "SPRITE frame found.\n");
        return AVERROR_PATCHWELCOME;
    }
    else
    {
        s->w = header.xsize;
        s->h = header.ysize;
        if (header.header_type < 2)
        {
            if ((s->w < 213) && (s->h >= 176))
            {
                s->flags |= FLAG_INTERPOLATED;
                av_log_ask_for_sample(s->avctx, "INTERPOLATION selected.\n");
            }
        }
    }
    if (header.compression >= 17)
    {
        av_log(s->avctx, AV_LOG_ERROR, "invalid compression type (%d)\n", header.compression);
        return AVERROR_INVALIDDATA;
    }
    if ((header.deltaset != s->last_deltaset) || (header.vectable != s->last_vectable))
        select_delta_tables(s, header.deltaset);
    if ((header.compression & 1) && header.header_type)
        sel_vector_table = pc_tbl2;
    else
    {
        if (header.vectable > 0 && header.vectable < 4)
            sel_vector_table = tables[header.vectable - 1];
        else
        {
            av_log(s->avctx, AV_LOG_ERROR, "invalid vector table id (%d)\n", header.vectable);
            return AVERROR_INVALIDDATA;
        }
    }
    if (compression_types[header.compression].algorithm == ALGO_RGB24H)
    {
        new_pix_fmt = AV_PIX_FMT_RGB32;
        width_shift = 1;
    }
    else
        new_pix_fmt = AV_PIX_FMT_RGB555;
    s->w >>= width_shift;
    if ((ret = av_image_check_size(s->w, s->h, 0, s->avctx)) < 0)
        return ret;
    if (s->w != s->avctx->width || s->h != s->avctx->height || new_pix_fmt != s->avctx->pix_fmt)
    {
        av_frame_unref(&s->frame);
        s->avctx->sample_aspect_ratio = (AVRational){1 << width_shift, 1};
        s->avctx->pix_fmt = new_pix_fmt;
        avcodec_set_dimensions(s->avctx, s->w, s->h);
        av_fast_malloc(&s->vert_pred, &s->vert_pred_size, s->avctx->width * sizeof(unsigned int));
    }
    s->mb_change_bits_row_size = ((s->avctx->width >> (2 - width_shift)) + 7) >> 3;
    if ((header.deltaset != s->last_deltaset) || (header.vectable != s->last_vectable))
    {
        if (compression_types[header.compression].algorithm == ALGO_RGB24H)
            gen_vector_table24(s, sel_vector_table);
        else if (s->avctx->pix_fmt == AV_PIX_FMT_RGB555)
            gen_vector_table15(s, sel_vector_table);
        else
            gen_vector_table16(s, sel_vector_table);
    }
    s->mb_change_bits = s->buf + header.header_size;
    if (s->flags & FLAG_KEYFRAME)
    {
        s->index_stream = s->mb_change_bits;
    }
    else
    {
        s->index_stream = s->mb_change_bits + (s->mb_change_bits_row_size * (s->avctx->height >> 2));
    }
    s->index_stream_size = s->size - (s->index_stream - s->buf);
    s->last_deltaset = header.deltaset;
    s->last_vectable = header.vectable;
    s->compression = header.compression;
    s->block_width = compression_types[header.compression].block_width;
    s->block_height = compression_types[header.compression].block_height;
    s->block_type = compression_types[header.compression].block_type;
    if (s->avctx->debug & FF_DEBUG_PICT_INFO)
        av_log(s->avctx, AV_LOG_INFO, "tables: %d / %d c:%d %dx%d t:%d %s%s%s%s\n", s->last_deltaset, s->last_vectable, s->compression, s->block_width, s->block_height, s->block_type, s->flags & FLAG_KEYFRAME ? " KEY" : "", s->flags & FLAG_INTERFRAME ? " INTER" : "", s->flags & FLAG_SPRITE ? " SPRITE" : "", s->flags & FLAG_INTERPOLATED ? " INTERPOL" : "");
    return header.header_size;
}