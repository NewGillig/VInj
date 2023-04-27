static void setup_frame_size(VP9_COMMON *cm, struct vp9_read_bit_buffer *rb)
{
    int width, height;
    vp9_read_frame_size(rb, &width, &height);
    resize_context_buffers(cm, width, height);
    setup_display_size(cm, rb);
    if (vp9_realloc_frame_buffer(get_frame_new_buffer(cm), cm->width, cm->height, cm->subsampling_x, cm->subsampling_y, #if CONFIG_VP9_HIGHBITDEPTH cm->use_highbitdepth, #endif VP9_DEC_BORDER_IN_PIXELS, &cm->frame_bufs[cm->new_fb_idx].raw_frame_buffer, cm->get_fb_cb, cm->cb_priv))
    {
        vpx_internal_error(&cm->error, VPX_CODEC_MEM_ERROR, "Failed to allocate frame buffer");
    }
}