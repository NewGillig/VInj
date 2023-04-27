void ff_init_qscale_tab(MpegEncContext *s)
{
    int8_t *const qscale_table = s->current_picture.qscale_table;
    int i;
    for (i = 0; i < s->mb_num; i++)
    {
        unsigned int lam = s->lambda_table[s->mb_index2xy[i]];
        int qp = (lam * 139 + FF_LAMBDA_SCALE * 64) >> (FF_LAMBDA_SHIFT + 7);
        qscale_table[s->mb_index2xy[i]] = av_clip(qp, s->avctx->qmin, s->avctx->qmax);
    }
}