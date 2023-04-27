void vp9_update_mv_count(VP9_COMMON *cm, const MACROBLOCKD *xd)
{
    const MODE_INFO *mi = xd->mi[0];
    const MB_MODE_INFO *const mbmi = &mi->mbmi;
    if (mbmi->sb_type < BLOCK_8X8)
    {
        const int num_4x4_w = num_4x4_blocks_wide_lookup[mbmi->sb_type];
        const int num_4x4_h = num_4x4_blocks_high_lookup[mbmi->sb_type];
        int idx, idy;
        for (idy = 0; idy < 2; idy += num_4x4_h)
        {
            for (idx = 0; idx < 2; idx += num_4x4_w)
            {
                const int i = idy * 2 + idx;
                if (mi->bmi[i].as_mode == NEWMV)
                    inc_mvs(mbmi, mi->bmi[i].as_mv, &cm->counts.mv);
            }
        }
    }
    else
    {
        if (mbmi->mode == NEWMV)
            inc_mvs(mbmi, mbmi->mv, &cm->counts.mv);
    }
}