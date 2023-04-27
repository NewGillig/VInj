int get_vp9_frame_buffer(void *cb_priv, size_t min_size, vpx_codec_frame_buffer_t *fb)
{
    int i;
    struct ExternalFrameBufferList *const ext_fb_list = (struct ExternalFrameBufferList *)cb_priv;
    if (ext_fb_list == NULL)
        return -1;
    for (i = 0; i < ext_fb_list->num_external_frame_buffers; ++i)
    {
        if (!ext_fb_list->ext_fb[i].in_use)
            break;
    }
    if (i == ext_fb_list->num_external_frame_buffers)
        return -1;
    if (ext_fb_list->ext_fb[i].size < min_size)
    {
        free(ext_fb_list->ext_fb[i].data);
        ext_fb_list->ext_fb[i].data = (uint8_t *)malloc(min_size);
        if (!ext_fb_list->ext_fb[i].data)
            return -1;
        ext_fb_list->ext_fb[i].size = min_size;
    }
    fb->data = ext_fb_list->ext_fb[i].data;
    fb->size = ext_fb_list->ext_fb[i].size;
    ext_fb_list->ext_fb[i].in_use = 1;
    fb->priv = &ext_fb_list->ext_fb[i];
    return 0;
}