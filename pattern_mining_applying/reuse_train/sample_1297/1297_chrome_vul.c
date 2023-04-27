static void write_ref_frames(const VP9_COMMON *cm, const MACROBLOCKD *xd, vp9_writer *w)
{
    const MB_MODE_INFO *const mbmi = &xd->mi[0]->mbmi;
    const int is_compound = has_second_ref(mbmi);
    const int segment_id = mbmi->segment_id;
    if (vp9_segfeature_active(&cm->seg, segment_id, SEG_LVL_REF_FRAME))
    {
        assert(!is_compound);
        assert(mbmi->ref_frame[0] == vp9_get_segdata(&cm->seg, segment_id, SEG_LVL_REF_FRAME));
    }
    else
    {
        if (cm->reference_mode == REFERENCE_MODE_SELECT)
        {
            vp9_write(w, is_compound, vp9_get_reference_mode_prob(cm, xd));
        }
        else
        {
            assert(!is_compound == (cm->reference_mode == SINGLE_REFERENCE));
        }
        if (is_compound)
        {
            vp9_write(w, mbmi->ref_frame[0] == GOLDEN_FRAME, vp9_get_pred_prob_comp_ref_p(cm, xd));
        }
        else
        {
            const int bit0 = mbmi->ref_frame[0] != LAST_FRAME;
            vp9_write(w, bit0, vp9_get_pred_prob_single_ref_p1(cm, xd));
            if (bit0)
            {
                const int bit1 = mbmi->ref_frame[0] != GOLDEN_FRAME;
                vp9_write(w, bit1, vp9_get_pred_prob_single_ref_p2(cm, xd));
            }
        }
    }
}