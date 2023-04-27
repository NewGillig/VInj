static void set_ref(VP9_COMMON *const cm, MACROBLOCKD *const xd, int idx, int mi_row, int mi_col)
{
    MB_MODE_INFO *const mbmi = &xd->mi[0].src_mi->mbmi;
    RefBuffer *ref_buffer = &cm->frame_refs[mbmi->ref_frame[idx] - LAST_FRAME];
    xd->block_refs[idx] = ref_buffer;
    if (!vp9_is_valid_scale(&ref_buffer->sf))
        vpx_internal_error(&cm->error, VPX_CODEC_UNSUP_BITSTREAM, "Invalid scale factors");
    if (ref_buffer->buf->corrupted)
        vpx_internal_error(&cm->error, VPX_CODEC_CORRUPT_FRAME, "Block reference is corrupt");
    vp9_setup_pre_planes(xd, idx, ref_buffer->buf, mi_row, mi_col, &ref_buffer->sf);
    xd->corrupted |= ref_buffer->buf->corrupted;
}