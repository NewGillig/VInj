void vp9_init_layer_context(VP9_COMP *const cpi)
{
    SVC *const svc = &cpi->svc;
    const VP9EncoderConfig *const oxcf = &cpi->oxcf;
    int layer;
    int layer_end;
    int alt_ref_idx = svc->number_spatial_layers;
    svc->spatial_layer_id = 0;
    svc->temporal_layer_id = 0;
    if (svc->number_temporal_layers > 1 && cpi->oxcf.rc_mode == VPX_CBR)
    {
        layer_end = svc->number_temporal_layers;
    }
    else
    {
        layer_end = svc->number_spatial_layers;
    }
    for (layer = 0; layer < layer_end; ++layer)
    {
        LAYER_CONTEXT *const lc = &svc->layer_context[layer];
        RATE_CONTROL *const lrc = &lc->rc;
        int i;
        lc->current_video_frame_in_layer = 0;
        lc->layer_size = 0;
        lc->frames_from_key_frame = 0;
        lc->last_frame_type = FRAME_TYPES;
        lrc->ni_av_qi = oxcf->worst_allowed_q;
        lrc->total_actual_bits = 0;
        lrc->total_target_vs_actual = 0;
        lrc->ni_tot_qi = 0;
        lrc->tot_q = 0.0;
        lrc->avg_q = 0.0;
        lrc->ni_frames = 0;
        lrc->decimation_count = 0;
        lrc->decimation_factor = 0;
        for (i = 0; i < RATE_FACTOR_LEVELS; ++i)
        {
            lrc->rate_correction_factors[i] = 1.0;
        }
        if (svc->number_temporal_layers > 1 && cpi->oxcf.rc_mode == VPX_CBR)
        {
            lc->target_bandwidth = oxcf->ts_target_bitrate[layer];
            lrc->last_q[INTER_FRAME] = oxcf->worst_allowed_q;
            lrc->avg_frame_qindex[INTER_FRAME] = oxcf->worst_allowed_q;
        }
        else
        {
            lc->target_bandwidth = oxcf->ss_target_bitrate[layer];
            lrc->last_q[KEY_FRAME] = oxcf->best_allowed_q;
            lrc->last_q[INTER_FRAME] = oxcf->best_allowed_q;
            lrc->avg_frame_qindex[KEY_FRAME] = (oxcf->worst_allowed_q + oxcf->best_allowed_q) / 2;
            lrc->avg_frame_qindex[INTER_FRAME] = (oxcf->worst_allowed_q + oxcf->best_allowed_q) / 2;
            if (oxcf->ss_play_alternate[layer])
                lc->alt_ref_idx = alt_ref_idx++;
            else
                lc->alt_ref_idx = -1;
            lc->gold_ref_idx = -1;
        }
        lrc->buffer_level = oxcf->starting_buffer_level_ms * lc->target_bandwidth / 1000;
        lrc->bits_off_target = lrc->buffer_level;
    }
    if (!(svc->number_temporal_layers > 1 && cpi->oxcf.rc_mode == VPX_CBR) && alt_ref_idx < REF_FRAMES)
        svc->layer_context[0].gold_ref_idx = alt_ref_idx;
}