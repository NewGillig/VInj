static void parse_mb_skip(Wmv2Context *w)
{
    int mb_x, mb_y;
    MpegEncContext *const s = &w->s;
    uint32_t *const mb_type = s->current_picture_ptr->mb_type;
    w->skip_type = get_bits(&s->gb, 2);
    switch (w->skip_type)
    {
    case SKIP_TYPE_NONE:
        for (mb_y = 0; mb_y < s->mb_height; mb_y++)
        {
            for (mb_x = 0; mb_x < s->mb_width; mb_x++)
            {
                mb_type[mb_y * s->mb_stride + mb_x] = MB_TYPE_16x16 | MB_TYPE_L0;
            }
        }
        break;
    case SKIP_TYPE_MPEG:
        for (mb_y = 0; mb_y < s->mb_height; mb_y++)
        {
            for (mb_x = 0; mb_x < s->mb_width; mb_x++)
            {
                mb_type[mb_y * s->mb_stride + mb_x] = (get_bits1(&s->gb) ? MB_TYPE_SKIP : 0) | MB_TYPE_16x16 | MB_TYPE_L0;
            }
        }
        break;
    case SKIP_TYPE_ROW:
        for (mb_y = 0; mb_y < s->mb_height; mb_y++)
        {
            if (get_bits1(&s->gb))
            {
                for (mb_x = 0; mb_x < s->mb_width; mb_x++)
                {
                    mb_type[mb_y * s->mb_stride + mb_x] = MB_TYPE_SKIP | MB_TYPE_16x16 | MB_TYPE_L0;
                }
            }
            else
            {
                for (mb_x = 0; mb_x < s->mb_width; mb_x++)
                {
                    mb_type[mb_y * s->mb_stride + mb_x] = (get_bits1(&s->gb) ? MB_TYPE_SKIP : 0) | MB_TYPE_16x16 | MB_TYPE_L0;
                }
            }
        }
        break;
    case SKIP_TYPE_COL:
        for (mb_x = 0; mb_x < s->mb_width; mb_x++)
        {
            if (get_bits1(&s->gb))
            {
                for (mb_y = 0; mb_y < s->mb_height; mb_y++)
                {
                    mb_type[mb_y * s->mb_stride + mb_x] = MB_TYPE_SKIP | MB_TYPE_16x16 | MB_TYPE_L0;
                }
            }
            else
            {
                for (mb_y = 0; mb_y < s->mb_height; mb_y++)
                {
                    mb_type[mb_y * s->mb_stride + mb_x] = (get_bits1(&s->gb) ? MB_TYPE_SKIP : 0) | MB_TYPE_16x16 | MB_TYPE_L0;
                }
            }
        }
        break;
    }
}