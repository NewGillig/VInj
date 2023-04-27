void vp9_change_config(struct VP9_COMP *cpi, const VP9EncoderConfig *oxcf)
{
    VP9_COMMON *const cm = &cpi->common;
    RATE_CONTROL *const rc = &cpi->rc;
    if (cm->profile != oxcf->profile)
        cm->profile = oxcf->profile;
    cm->bit_depth = oxcf->bit_depth;
    if (cm->profile <= PROFILE_1)
        assert(cm->bit_depth == VPX_BITS_8);
    else
        assert(cm->bit_depth > VPX_BITS_8);
    cpi->oxcf = *oxcf;
    cpi->mb.e_mbd.bd = (int)cm->bit_depth;
}