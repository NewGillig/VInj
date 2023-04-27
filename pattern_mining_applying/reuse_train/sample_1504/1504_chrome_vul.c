void vp9_rc_init(const VP9EncoderConfig *oxcf, int pass, RATE_CONTROL *rc)
{
    int i;
    if (pass == 0 && oxcf->rc_mode == VPX_CBR)
    {
        rc->avg_frame_qindex[KEY_FRAME] = oxcf->worst_allowed_q;
        rc->avg_frame_qindex[INTER_FRAME] = oxcf->worst_allowed_q;
    }
    else
    {
        rc->avg_frame_qindex[KEY_FRAME] = (oxcf->worst_allowed_q + oxcf->best_allowed_q) / 2;
        rc->avg_frame_qindex[INTER_FRAME] = (oxcf->worst_allowed_q + oxcf->best_allowed_q) / 2;
    }
    rc->last_q[KEY_FRAME] = oxcf->best_allowed_q;
    rc->last_q[INTER_FRAME] = oxcf->best_allowed_q;
    rc->buffer_level = rc->starting_buffer_level;
    rc->bits_off_target = rc->starting_buffer_level;
    rc->rolling_target_bits = rc->avg_frame_bandwidth;
    rc->rolling_actual_bits = rc->avg_frame_bandwidth;
    rc->long_rolling_target_bits = rc->avg_frame_bandwidth;
    rc->long_rolling_actual_bits = rc->avg_frame_bandwidth;
    rc->total_actual_bits = 0;
    rc->total_target_bits = 0;
    rc->total_target_vs_actual = 0;
    rc->baseline_gf_interval = DEFAULT_GF_INTERVAL;
    rc->frames_since_key = 8;
    rc->this_key_frame_forced = 0;
    rc->next_key_frame_forced = 0;
    rc->source_alt_ref_pending = 0;
    rc->source_alt_ref_active = 0;
    rc->frames_till_gf_update_due = 0;
    rc->ni_av_qi = oxcf->worst_allowed_q;
    rc->ni_tot_qi = 0;
    rc->ni_frames = 0;
    rc->tot_q = 0.0;
    rc->avg_q = vp9_convert_qindex_to_q(oxcf->worst_allowed_q);
    for (i = 0; i < RATE_FACTOR_LEVELS; ++i)
    {
        rc->rate_correction_factors[i] = 1.0;
    }
}