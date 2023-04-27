static int h261_decode_mb_skipped(H261Context *h, int mba1, int mba2)
{
    MpegEncContext *const s = &h->s;
    int i;
    s->mb_intra = 0;
    for (i = mba1; i < mba2; i++)
    {
        int j, xy;
        s->mb_x = ((h->gob_number - 1) % 2) * 11 + i % 11;
        s->mb_y = ((h->gob_number - 1) / 2) * 3 + i / 11;
        xy = s->mb_x + s->mb_y * s->mb_stride;
        ff_init_block_index(s);
        ff_update_block_index(s);
        for (j = 0; j < 6; j++)
            s->block_last_index[j] = -1;
        s->mv_dir = MV_DIR_FORWARD;
        s->mv_type = MV_TYPE_16X16;
        s->current_picture.mb_type[xy] = MB_TYPE_SKIP | MB_TYPE_16x16 | MB_TYPE_L0;
        s->mv[0][0][0] = 0;
        s->mv[0][0][1] = 0;
        s->mb_skipped = 1;
        h->mtype &= ~MB_TYPE_H261_FIL;
        ff_MPV_decode_mb(s, s->block);
    }
    return 0;
}