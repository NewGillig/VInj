static void fill_vaapi_pic(VAPictureH264 *va_pic, Picture *pic, int pic_structure)
{
    if (pic_structure == 0)
        pic_structure = pic->reference;
    pic_structure &= PICT_FRAME;
    va_pic->picture_id = ff_vaapi_get_surface_id(pic);
    va_pic->frame_idx = pic->long_ref ? pic->pic_id : pic->frame_num;
    va_pic->flags = 0;
    if (pic_structure != PICT_FRAME)
        va_pic->flags |= (pic_structure & PICT_TOP_FIELD) ? VA_PICTURE_H264_TOP_FIELD : VA_PICTURE_H264_BOTTOM_FIELD;
    if (pic->reference)
        va_pic->flags |= pic->long_ref ? VA_PICTURE_H264_LONG_TERM_REFERENCE : VA_PICTURE_H264_SHORT_TERM_REFERENCE;
    va_pic->TopFieldOrderCnt = 0;
    if (pic->field_poc[0] != INT_MAX)
        va_pic->TopFieldOrderCnt = pic->field_poc[0];
    va_pic->BottomFieldOrderCnt = 0;
    if (pic->field_poc[1] != INT_MAX)
        va_pic->BottomFieldOrderCnt = pic->field_poc[1];
}