int mem_get_bits_rectangle(gx_device *dev, const gs_int_rect *prect, gs_get_bits_params_t *params, gs_int_rect **unread)
{
    gx_device_memory *const mdev = (gx_device_memory *)dev;
    gs_get_bits_options_t options = params->options;
    int x = prect->p.x, w = prect->q.x - x, y = prect->p.y, h = prect->q.y - y;
    if (options == 0)
    {
        params->options = (GB_ALIGN_STANDARD | GB_ALIGN_ANY) | (GB_RETURN_COPY | GB_RETURN_POINTER) | (GB_OFFSET_0 | GB_OFFSET_SPECIFIED | GB_OFFSET_ANY) | (GB_RASTER_STANDARD | GB_RASTER_SPECIFIED | GB_RASTER_ANY) | GB_PACKING_CHUNKY | GB_COLORS_NATIVE | GB_ALPHA_NONE;
        return_error(gs_error_rangecheck);
    }
    if ((w <= 0) | (h <= 0))
    {
        if ((w | h) < 0)
            return_error(gs_error_rangecheck);
        return 0;
    }
    if (x < 0 || w > dev->width - x || y < 0 || h > dev->height - y)
        return_error(gs_error_rangecheck);
    {
        gs_get_bits_params_t copy_params;
        byte **base = &scan_line_base(mdev, y);
        int code;
        copy_params.options = GB_COLORS_NATIVE | GB_PACKING_CHUNKY | GB_ALPHA_NONE | (mdev->raster == bitmap_raster(mdev->width * mdev->color_info.depth) ? GB_RASTER_STANDARD : GB_RASTER_SPECIFIED);
        copy_params.raster = mdev->raster;
        code = gx_get_bits_return_pointer(dev, x, h, params, &copy_params, base);
        if (code >= 0)
            return code;
        return gx_get_bits_copy(dev, x, w, h, params, &copy_params, *base, gx_device_raster(dev, true));
    }
}