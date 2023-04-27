void PNGAPI png_set_PLTE(png_structp png_ptr, png_infop info_ptr, png_colorp palette, int num_palette)
{
    png_uint_32 max_palette_length;
    png_debug1(1, "in %s storage function", "PLTE");
    if (png_ptr == NULL || info_ptr == NULL)
        return;
    max_palette_length = (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE) ? (1 << png_ptr->bit_depth) : PNG_MAX_PALETTE_LENGTH;
    if (num_palette < 0 || num_palette > (int)max_palette_length)
    {
        if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
            png_error(png_ptr, "Invalid palette length");
        else
        {
            png_warning(png_ptr, "Invalid palette length");
            return;
        }
    }
    png_memcpy(png_ptr->palette, palette, num_palette * png_sizeof(png_color));
    info_ptr->palette = png_ptr->palette;
    info_ptr->num_palette = png_ptr->num_palette = (png_uint_16)num_palette;
}