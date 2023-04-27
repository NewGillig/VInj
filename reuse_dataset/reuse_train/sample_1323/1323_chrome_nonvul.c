void vp9_tokenize_sb(VP9_COMP *cpi, TOKENEXTRA **t, int dry_run, BLOCK_SIZE bsize)
{
    VP9_COMMON *const cm = &cpi->common;
    MACROBLOCKD *const xd = &cpi->mb.e_mbd;
    MB_MODE_INFO *const mbmi = &xd->mi[0].src_mi->mbmi;
    TOKENEXTRA *t_backup = *t;
    const int ctx = vp9_get_skip_context(xd);
    const int skip_inc = !vp9_segfeature_active(&cm->seg, mbmi->segment_id, SEG_LVL_SKIP);
    struct tokenize_b_args arg = {cpi, xd, t};
    if (mbmi->skip)
    {
        if (!dry_run)
            cm->counts.skip[ctx][1] += skip_inc;
        reset_skip_context(xd, bsize);
        if (dry_run)
            *t = t_backup;
        return;
    }
    if (!dry_run)
    {
        cm->counts.skip[ctx][0] += skip_inc;
        vp9_foreach_transformed_block(xd, bsize, tokenize_b, &arg);
    }
    else
    {
        vp9_foreach_transformed_block(xd, bsize, set_entropy_context_b, &arg);
        *t = t_backup;
    }
}