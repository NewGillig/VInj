static int decode_cabac_field_decoding_flag(H264Context *h)
{
    const long mbb_xy = h->mb_xy - 2L * h->mb_stride;
    unsigned long ctx = 0;
    ctx += h->mb_field_decoding_flag & !!h->mb_x;
    ctx += (h->cur_pic.mb_type[mbb_xy] >> 7) & (h->slice_table[mbb_xy] == h->slice_num);
    return get_cabac_noinline(&h->cabac, &(h->cabac_state + 70)[ctx]);
}