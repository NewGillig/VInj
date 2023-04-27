void vp9_append_sub8x8_mvs_for_idx(VP9_COMMON *cm, MACROBLOCKD *xd, const TileInfo *const tile, int block, int ref, int mi_row, int mi_col, int_mv *nearest, int_mv *near)
{
    int_mv mv_list[MAX_MV_REF_CANDIDATES];
    MODE_INFO *const mi = xd->mi[0];
    b_mode_info *bmi = mi->bmi;
    int n;
    assert(MAX_MV_REF_CANDIDATES == 2);
    find_mv_refs_idx(cm, xd, tile, mi, mi->mbmi.ref_frame[ref], mv_list, block, mi_row, mi_col);
    near->as_int = 0;
    switch (block)
    {
    case 0:
        nearest->as_int = mv_list[0].as_int;
        near->as_int = mv_list[1].as_int;
        break;
    case 1:
    case 2:
        nearest->as_int = bmi[0].as_mv[ref].as_int;
        for (n = 0; n < MAX_MV_REF_CANDIDATES; ++n)
            if (nearest->as_int != mv_list[n].as_int)
            {
                near->as_int = mv_list[n].as_int;
                break;
            }
        break;
    case 3:
    {
        int_mv candidates[2 + MAX_MV_REF_CANDIDATES];
        candidates[0] = bmi[1].as_mv[ref];
        candidates[1] = bmi[0].as_mv[ref];
        candidates[2] = mv_list[0];
        candidates[3] = mv_list[1];
        nearest->as_int = bmi[2].as_mv[ref].as_int;
        for (n = 0; n < 2 + MAX_MV_REF_CANDIDATES; ++n)
            if (nearest->as_int != candidates[n].as_int)
            {
                near->as_int = candidates[n].as_int;
                break;
            }
        break;
    }
    default:
        assert("Invalid block index.");
    }
}