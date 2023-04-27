static int64_t rd_sbuv_dcpred(const VP9_COMP *cpi, MACROBLOCK *x, int *rate, int *rate_tokenonly, int64_t *distortion, int *skippable, BLOCK_SIZE bsize)
{
    const VP9_COMMON *cm = &cpi->common;
    int64_t unused;
    x->e_mbd.mi[0].src_mi->mbmi.uv_mode = DC_PRED;
    super_block_uvrd(cpi, x, rate_tokenonly, distortion, skippable, &unused, bsize, INT64_MAX);
    *rate = *rate_tokenonly + cpi->intra_uv_mode_cost[cm->frame_type][DC_PRED];
    return RDCOST(x->rdmult, x->rddiv, *rate, *distortion);
}