static int rv34_set_deblock_coef(RV34DecContext *r)
{
    MpegEncContext *s = &r->s;
    int hmvmask = 0, vmvmask = 0, i, j;
    int midx = s->mb_x * 2 + s->mb_y * 2 * s->b8_stride;
    int16_t(*motion_val)[2] = &s->current_picture_ptr->motion_val[0][midx];
    for (j = 0; j < 16; j += 8)
    {
        for (i = 0; i < 2; i++)
        {
            if (is_mv_diff_gt_3(motion_val + i, 1))
                vmvmask |= 0x11 << (j + i * 2);
            if ((j || s->mb_y) && is_mv_diff_gt_3(motion_val + i, s->b8_stride))
                hmvmask |= 0x03 << (j + i * 2);
        }
        motion_val += s->b8_stride;
    }
    if (s->first_slice_line)
        hmvmask &= ~0x000F;
    if (!s->mb_x)
        vmvmask &= ~0x1111;
    if (r->rv30)
    {
        vmvmask |= (vmvmask & 0x4444) >> 1;
        hmvmask |= (hmvmask & 0x0F00) >> 4;
        if (s->mb_x)
            r->deblock_coefs[s->mb_x - 1 + s->mb_y * s->mb_stride] |= (vmvmask & 0x1111) << 3;
        if (!s->first_slice_line)
            r->deblock_coefs[s->mb_x + (s->mb_y - 1) * s->mb_stride] |= (hmvmask & 0xF) << 12;
    }
    return hmvmask | vmvmask;
}