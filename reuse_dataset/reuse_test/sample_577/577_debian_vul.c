static int rv34_decode_slice(RV34DecContext *r, int end, const uint8_t *buf, int buf_size)
{
    MpegEncContext *s = &r->s;
    GetBitContext *gb = &s->gb;
    int mb_pos, slice_type;
    int res;
    init_get_bits(&r->s.gb, buf, buf_size * 8);
    res = r->parse_slice_header(r, gb, &r->si);
    if (res < 0)
    {
        av_log(s->avctx, AV_LOG_ERROR, "Incorrect or unknown slice header\n");
        return -1;
    }
    slice_type = r->si.type ? r->si.type : AV_PICTURE_TYPE_I;
    if (slice_type != s->pict_type)
    {
        av_log(s->avctx, AV_LOG_ERROR, "Slice type mismatch\n");
        return AVERROR_INVALIDDATA;
    }
    r->si.end = end;
    s->qscale = r->si.quant;
    s->mb_num_left = r->si.end - r->si.start;
    r->s.mb_skip_run = 0;
    mb_pos = s->mb_x + s->mb_y * s->mb_width;
    if (r->si.start != mb_pos)
    {
        av_log(s->avctx, AV_LOG_ERROR, "Slice indicates MB offset %d, got %d\n", r->si.start, mb_pos);
        s->mb_x = r->si.start % s->mb_width;
        s->mb_y = r->si.start / s->mb_width;
    }
    memset(r->intra_types_hist, -1, r->intra_types_stride * 4 * 2 * sizeof(*r->intra_types_hist));
    s->first_slice_line = 1;
    s->resync_mb_x = s->mb_x;
    s->resync_mb_y = s->mb_y;
    ff_init_block_index(s);
    while (!check_slice_end(r, s))
    {
        ff_update_block_index(s);
        if (r->si.type)
            res = rv34_decode_inter_macroblock(r, r->intra_types + s->mb_x * 4 + 4);
        else
            res = rv34_decode_intra_macroblock(r, r->intra_types + s->mb_x * 4 + 4);
        if (res < 0)
        {
            ff_er_add_slice(&s->er, s->resync_mb_x, s->resync_mb_y, s->mb_x - 1, s->mb_y, ER_MB_ERROR);
            return -1;
        }
        if (++s->mb_x == s->mb_width)
        {
            s->mb_x = 0;
            s->mb_y++;
            ff_init_block_index(s);
            memmove(r->intra_types_hist, r->intra_types, r->intra_types_stride * 4 * sizeof(*r->intra_types_hist));
            memset(r->intra_types, -1, r->intra_types_stride * 4 * sizeof(*r->intra_types_hist));
            if (r->loop_filter && s->mb_y >= 2)
                r->loop_filter(r, s->mb_y - 2);
            if (HAVE_THREADS && (s->avctx->active_thread_type & FF_THREAD_FRAME))
                ff_thread_report_progress(&s->current_picture_ptr->f, s->mb_y - 2, 0);
        }
        if (s->mb_x == s->resync_mb_x)
            s->first_slice_line = 0;
        s->mb_num_left--;
    }
    ff_er_add_slice(&s->er, s->resync_mb_x, s->resync_mb_y, s->mb_x - 1, s->mb_y, ER_MB_END);
    return s->mb_y == s->mb_height;
}