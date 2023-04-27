void vp9_pick_filter_level(const YV12_BUFFER_CONFIG *sd, VP9_COMP *cpi, LPF_PICK_METHOD method)
{
    VP9_COMMON *const cm = &cpi->common;
    struct loopfilter *const lf = &cm->lf;
    lf->sharpness_level = cm->frame_type == KEY_FRAME ? 0 : cpi->oxcf.sharpness;
    if (method == LPF_PICK_MINIMAL_LPF && lf->filter_level)
    {
        lf->filter_level = 0;
    }
    else if (method >= LPF_PICK_FROM_Q)
    {
        const int min_filter_level = 0;
        const int max_filter_level = get_max_filter_level(cpi);
        const int q = vp9_ac_quant(cm->base_qindex, 0, cm->bit_depth);
        int filt_guess = ROUND_POWER_OF_TWO(q * 20723 + 1015158, 18);
        if (cm->frame_type == KEY_FRAME)
            filt_guess -= 4;
        lf->filter_level = clamp(filt_guess, min_filter_level, max_filter_level);
    }
    else
    {
        lf->filter_level = search_filter_level(sd, cpi, method == LPF_PICK_FROM_SUBIMAGE);
    }
}