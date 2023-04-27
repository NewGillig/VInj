void vp9_twopass_postencode_update(VP9_COMP *cpi)
{
    TWO_PASS *const twopass = &cpi->twopass;
    RATE_CONTROL *const rc = &cpi->rc;
    const int bits_used = rc->base_frame_target;
    rc->vbr_bits_off_target += rc->base_frame_target - rc->projected_frame_size;
    twopass->bits_left = MAX(twopass->bits_left - bits_used, 0);
    if (cpi->common.frame_type != KEY_FRAME && !vp9_is_upper_layer_key_frame(cpi))
    {
        twopass->kf_group_bits -= bits_used;
        twopass->last_kfgroup_zeromotion_pct = twopass->kf_zeromotion_pct;
    }
    twopass->kf_group_bits = MAX(twopass->kf_group_bits, 0);
    ++twopass->gf_group.index;
}