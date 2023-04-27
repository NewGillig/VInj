void vp9_init_plane_quantizers(VP9_COMP *cpi, MACROBLOCK *x)
{
    const VP9_COMMON *const cm = &cpi->common;
    MACROBLOCKD *const xd = &x->e_mbd;
    QUANTS *const quants = &cpi->quants;
    const int segment_id = xd->mi[0]->mbmi.segment_id;
    const int qindex = vp9_get_qindex(&cm->seg, segment_id, cm->base_qindex);
    const int rdmult = vp9_compute_rd_mult(cpi, qindex + cm->y_dc_delta_q);
    const int zbin = cpi->zbin_mode_boost;
    int i;
    x->plane[0].quant = quants->y_quant[qindex];
    x->plane[0].quant_fp = quants->y_quant_fp[qindex];
    x->plane[0].round_fp = quants->y_round_fp[qindex];
    x->plane[0].quant_shift = quants->y_quant_shift[qindex];
    x->plane[0].zbin = quants->y_zbin[qindex];
    x->plane[0].round = quants->y_round[qindex];
    x->plane[0].quant_thred[0] = cm->y_dequant[qindex][0] * cm->y_dequant[qindex][0];
    x->plane[0].quant_thred[1] = cm->y_dequant[qindex][1] * cm->y_dequant[qindex][1];
    x->plane[0].zbin_extra = (int16_t)((cm->y_dequant[qindex][1] * zbin) >> 7);
    xd->plane[0].dequant = cm->y_dequant[qindex];
    for (i = 1; i < 3; i++)
    {
        x->plane[i].quant = quants->uv_quant[qindex];
        x->plane[i].quant_fp = quants->uv_quant_fp[qindex];
        x->plane[i].round_fp = quants->uv_round_fp[qindex];
        x->plane[i].quant_shift = quants->uv_quant_shift[qindex];
        x->plane[i].zbin = quants->uv_zbin[qindex];
        x->plane[i].round = quants->uv_round[qindex];
        x->plane[i].quant_thred[0] = cm->y_dequant[qindex][0] * cm->y_dequant[qindex][0];
        x->plane[i].quant_thred[1] = cm->y_dequant[qindex][1] * cm->y_dequant[qindex][1];
        x->plane[i].zbin_extra = (int16_t)((cm->uv_dequant[qindex][1] * zbin) >> 7);
        xd->plane[i].dequant = cm->uv_dequant[qindex];
    }
    x->skip_block = vp9_segfeature_active(&cm->seg, segment_id, SEG_LVL_SKIP);
    x->q_index = qindex;
    x->errorperbit = rdmult >> 6;
    x->errorperbit += (x->errorperbit == 0);
    vp9_initialize_me_consts(cpi, x->q_index);
}