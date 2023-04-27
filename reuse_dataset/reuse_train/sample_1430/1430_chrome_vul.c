VP9Decoder *vp9_decoder_create()
{
    VP9Decoder *const pbi = vpx_memalign(32, sizeof(*pbi));
    VP9_COMMON *const cm = pbi ? &pbi->common : NULL;
    if (!cm)
        return NULL;
    vp9_zero(*pbi);
    if (setjmp(cm->error.jmp))
    {
        cm->error.setjmp = 0;
        vp9_decoder_remove(pbi);
        return NULL;
    }
    cm->error.setjmp = 1;
    initialize_dec();
    vpx_memset(&cm->ref_frame_map, -1, sizeof(cm->ref_frame_map));
    cm->current_video_frame = 0;
    pbi->ready_for_new_data = 1;
    cm->bit_depth = VPX_BITS_8;
    vp9_init_dequantizer(cm);
    vp9_loop_filter_init(cm);
    cm->error.setjmp = 0;
    vp9_get_worker_interface()->init(&pbi->lf_worker);
    return pbi;
}