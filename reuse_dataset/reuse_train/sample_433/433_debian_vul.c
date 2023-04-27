void ff_clean_h263_qscales(MpegEncContext *s)
{
    int i;
    int8_t *const qscale_table = s->current_picture.f.qscale_table;
    ff_init_qscale_tab(s);
    for (i = 1; i < s->mb_num; i++)
    {
        if (qscale_table[s->mb_index2xy[i]] - qscale_table[s->mb_index2xy[i - 1]] > 2)
            qscale_table[s->mb_index2xy[i]] = qscale_table[s->mb_index2xy[i - 1]] + 2;
    }
    for (i = s->mb_num - 2; i >= 0; i--)
    {
        if (qscale_table[s->mb_index2xy[i]] - qscale_table[s->mb_index2xy[i + 1]] > 2)
            qscale_table[s->mb_index2xy[i]] = qscale_table[s->mb_index2xy[i + 1]] + 2;
    }
    if (s->codec_id != AV_CODEC_ID_H263P)
    {
        for (i = 1; i < s->mb_num; i++)
        {
            int mb_xy = s->mb_index2xy[i];
            if (qscale_table[mb_xy] != qscale_table[s->mb_index2xy[i - 1]] && (s->mb_type[mb_xy] & CANDIDATE_MB_TYPE_INTER4V))
            {
                s->mb_type[mb_xy] |= CANDIDATE_MB_TYPE_INTER;
            }
        }
    }
}