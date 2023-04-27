static void await_reference_row(Vp3DecodeContext *s, Vp3Fragment *fragment, int motion_y, int y)
{
    AVFrame *ref_frame;
    int ref_row;
    int border = motion_y & 1;
    if (fragment->coding_method == MODE_USING_GOLDEN || fragment->coding_method == MODE_GOLDEN_MV)
        ref_frame = &s->golden_frame;
    else
        ref_frame = &s->last_frame;
    ref_row = y + (motion_y >> 1);
    ref_row = FFMAX(FFABS(ref_row), ref_row + 8 + border);
    ff_thread_await_progress(ref_frame, ref_row, 0);
}