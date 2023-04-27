static void update_stats(VP9_COMMON *cm, const MACROBLOCK *x)
{
    const MACROBLOCKD *const xd = &x->e_mbd;
    const MODE_INFO *const mi = xd->mi[0].src_mi;
    const MB_MODE_INFO *const mbmi = &mi->mbmi;
    if (!frame_is_intra_only(cm))
    {
        const int seg_ref_active = vp9_segfeature_active(&cm->seg, mbmi->segment_id, SEG_LVL_REF_FRAME);
        if (!seg_ref_active)
        {
            FRAME_COUNTS *const counts = &cm->counts;
            const int inter_block = is_inter_block(mbmi);
            counts->intra_inter[vp9_get_intra_inter_context(xd)][inter_block]++;
            if (inter_block)
            {
                const MV_REFERENCE_FRAME ref0 = mbmi->ref_frame[0];
                if (cm->reference_mode == REFERENCE_MODE_SELECT)
                    counts->comp_inter[vp9_get_reference_mode_context(cm, xd)][has_second_ref(mbmi)]++;
                if (has_second_ref(mbmi))
                {
                    counts->comp_ref[vp9_get_pred_context_comp_ref_p(cm, xd)][ref0 == GOLDEN_FRAME]++;
                }
                else
                {
                    counts->single_ref[vp9_get_pred_context_single_ref_p1(xd)][0][ref0 != LAST_FRAME]++;
                    if (ref0 != LAST_FRAME)
                        counts->single_ref[vp9_get_pred_context_single_ref_p2(xd)][1][ref0 != GOLDEN_FRAME]++;
                }
            }
        }
    }
}