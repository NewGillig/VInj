void vp9_set_speed_features(VP9_COMP *cpi)
{
    SPEED_FEATURES *const sf = &cpi->sf;
    VP9_COMMON *const cm = &cpi->common;
    const VP9EncoderConfig *const oxcf = &cpi->oxcf;
    int i;
    sf->frame_parameter_update = 1;
    sf->mv.search_method = NSTEP;
    sf->recode_loop = ALLOW_RECODE;
    sf->mv.subpel_search_method = SUBPEL_TREE;
    sf->mv.subpel_iters_per_step = 2;
    sf->mv.subpel_force_stop = 0;
    sf->optimize_coefficients = !is_lossless_requested(&cpi->oxcf);
    sf->mv.reduce_first_step_size = 0;
    sf->mv.auto_mv_step_size = 0;
    sf->mv.fullpel_search_step_param = 6;
    sf->comp_inter_joint_search_thresh = BLOCK_4X4;
    sf->adaptive_rd_thresh = 0;
    sf->use_lastframe_partitioning = LAST_FRAME_PARTITION_OFF;
    sf->tx_size_search_method = USE_FULL_RD;
    sf->use_lp32x32fdct = 0;
    sf->adaptive_motion_search = 0;
    sf->adaptive_pred_interp_filter = 0;
    sf->adaptive_mode_search = 0;
    sf->cb_pred_filter_search = 0;
    sf->cb_partition_search = 0;
    sf->motion_field_mode_search = 0;
    sf->alt_ref_search_fp = 0;
    sf->use_quant_fp = 0;
    sf->reference_masking = 0;
    sf->partition_search_type = SEARCH_PARTITION;
    sf->less_rectangular_check = 0;
    sf->use_square_partition_only = 0;
    sf->auto_min_max_partition_size = NOT_IN_USE;
    sf->max_partition_size = BLOCK_64X64;
    sf->min_partition_size = BLOCK_4X4;
    sf->adjust_partitioning_from_last_frame = 0;
    sf->last_partitioning_redo_frequency = 4;
    sf->constrain_copy_partition = 0;
    sf->disable_split_mask = 0;
    sf->mode_search_skip_flags = 0;
    sf->force_frame_boost = 0;
    sf->max_delta_qindex = 0;
    sf->disable_filter_search_var_thresh = 0;
    sf->adaptive_interp_filter_search = 0;
    for (i = 0; i < TX_SIZES; i++)
    {
        sf->intra_y_mode_mask[i] = INTRA_ALL;
        sf->intra_uv_mode_mask[i] = INTRA_ALL;
    }
    sf->use_rd_breakout = 0;
    sf->skip_encode_sb = 0;
    sf->use_uv_intra_rd_estimate = 0;
    sf->allow_skip_recode = 0;
    sf->lpf_pick = LPF_PICK_FROM_FULL_IMAGE;
    sf->use_fast_coef_updates = TWO_LOOP;
    sf->use_fast_coef_costing = 0;
    sf->mode_skip_start = MAX_MODES;
    sf->use_nonrd_pick_mode = 0;
    for (i = 0; i < BLOCK_SIZES; ++i)
        sf->inter_mode_mask[i] = INTER_ALL;
    sf->max_intra_bsize = BLOCK_64X64;
    sf->reuse_inter_pred_sby = 0;
    sf->always_this_block_size = BLOCK_16X16;
    sf->search_type_check_frequency = 50;
    sf->encode_breakout_thresh = 0;
    sf->elevate_newmv_thresh = 0;
    sf->recode_tolerance = 25;
    sf->default_interp_filter = SWITCHABLE;
    sf->tx_size_search_breakout = 0;
    sf->partition_search_breakout_dist_thr = 0;
    sf->partition_search_breakout_rate_thr = 0;
    if (oxcf->mode == REALTIME)
        set_rt_speed_feature(cpi, sf, oxcf->speed, oxcf->content);
    else if (oxcf->mode == GOOD)
        set_good_speed_feature(cpi, cm, sf, oxcf->speed);
    cpi->full_search_sad = vp9_full_search_sad;
    cpi->diamond_search_sad = oxcf->mode == BEST ? vp9_full_range_search : vp9_diamond_search_sad;
    cpi->refining_search_sad = vp9_refining_search_sad;
    if (oxcf->pass == 1)
        sf->optimize_coefficients = 0;
    if (oxcf->pass == 0)
    {
        sf->recode_loop = DISALLOW_RECODE;
        sf->optimize_coefficients = 0;
    }
    if (sf->mv.subpel_search_method == SUBPEL_TREE)
    {
        cpi->find_fractional_mv_step = vp9_find_best_sub_pixel_tree;
    }
    else if (sf->mv.subpel_search_method == SUBPEL_TREE_PRUNED)
    {
        cpi->find_fractional_mv_step = vp9_find_best_sub_pixel_tree_pruned;
    }
    cpi->mb.optimize = sf->optimize_coefficients == 1 && oxcf->pass != 1;
    if (sf->disable_split_mask == DISABLE_ALL_SPLIT)
        sf->adaptive_pred_interp_filter = 0;
    if (!cpi->oxcf.frame_periodic_boost)
    {
        sf->max_delta_qindex = 0;
    }
    if (cpi->encode_breakout && oxcf->mode == REALTIME && sf->encode_breakout_thresh > cpi->encode_breakout)
        cpi->encode_breakout = sf->encode_breakout_thresh;
}