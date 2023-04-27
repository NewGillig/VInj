int vp9_get_frame_buffer(void *cb_priv, size_t min_size, vpx_codec_frame_buffer_t *fb)
{
    int i;
    InternalFrameBufferList *const int_fb_list = (InternalFrameBufferList *)cb_priv;
    if (int_fb_list == NULL)
        return -1;
    for (i = 0; i < int_fb_list->num_internal_frame_buffers; ++i)
    {
        if (!int_fb_list->int_fb[i].in_use)
            break;
    }
    if (i == int_fb_list->num_internal_frame_buffers)
        return -1;
    if (int_fb_list->int_fb[i].size < min_size)
    {
        int_fb_list->int_fb[i].data = (uint8_t *)vpx_realloc(int_fb_list->int_fb[i].data, min_size);
        if (!int_fb_list->int_fb[i].data)
            return -1;
        int_fb_list->int_fb[i].size = min_size;
    }
    fb->data = int_fb_list->int_fb[i].data;
    fb->size = int_fb_list->int_fb[i].size;
    int_fb_list->int_fb[i].in_use = 1;
    fb->priv = &int_fb_list->int_fb[i];
    return 0;
}