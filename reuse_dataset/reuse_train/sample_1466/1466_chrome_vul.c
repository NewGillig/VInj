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
    rc->baseline_gf_interval = DEFAULT_GF_INTERVAL;
    cpi->refresh_golden_frame = 0;
    cpi->refresh_last_frame = 1;
    cm->refresh_frame_context = 1;
    cm->reset_frame_context = 0;
    vp9_reset_segment_features(&cm->seg);
    vp9_set_high_precision_mv(cpi, 0);
    {
        int i;
        for (i = 0; i < MAX_SEGMENTS; i++)
            cpi->segment_encode_breakout[i] = cpi->oxcf.encode_breakout;
    }
    cpi->encode_breakout = cpi->oxcf.encode_breakout;
    set_rc_buffer_sizes(rc, &cpi->oxcf);
    rc->bits_off_target = MIN(rc->bits_off_target, rc->maximum_buffer_size);
    rc->buffer_level = MIN(rc->buffer_level, rc->maximum_buffer_size);
    vp9_new_framerate(cpi, cpi->framerate);
    rc->worst_quality = cpi->oxcf.worst_allowed_q;
    rc->best_quality = cpi->oxcf.best_allowed_q;
    cm->interp_filter = cpi->sf.default_interp_filter;
    cm->display_width = cpi->oxcf.width;
    cm->display_height = cpi->oxcf.height;
    if (cpi->initial_width)
    {
        assert(cm->width <= cpi->initial_width);
        assert(cm->height <= cpi->initial_height);
    }
    update_frame_size(cpi);
    if ((cpi->svc.number_temporal_layers > 1 && cpi->oxcf.rc_mode == VPX_CBR) || ((cpi->svc.number_temporal_layers > 1 || cpi->svc.number_spatial_layers > 1) && cpi->oxcf.pass == 2))
    {
        vp9_update_layer_context_change_config(cpi, (int)cpi->oxcf.target_bandwidth);
    }
    cpi->alt_ref_source = NULL;
    rc->is_src_frame_alt_ref = 0;
    cpi->last_frame_distortion = 0;
    cpi->ext_refresh_frame_flags_pending = 0;
    cpi->ext_refresh_frame_context_pending = 0;
    vp9_denoiser_alloc(&(cpi->denoiser), cm->width, cm->height, cm->subsampling_x, cm->subsampling_y, #if CONFIG_VP9_HIGHBITDEPTH cm->use_highbitdepth, #endif VP9_ENC_BORDER_IN_PIXELS);
}