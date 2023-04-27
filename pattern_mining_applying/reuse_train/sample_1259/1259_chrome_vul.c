static int read_intra_segment_id(VP9_COMMON *const cm, MACROBLOCKD *const xd, int mi_row, int mi_col, vp9_reader *r)
{
    struct segmentation *const seg = &cm->seg;
    const BLOCK_SIZE bsize = xd->mi[0]->mbmi.sb_type;
    int segment_id;
    if (!seg->enabled)
        return 0;
    if (!seg->update_map)
        return 0;
    segment_id = read_segment_id(r, seg);
    set_segment_id(cm, bsize, mi_row, mi_col, segment_id);
    return segment_id;
}