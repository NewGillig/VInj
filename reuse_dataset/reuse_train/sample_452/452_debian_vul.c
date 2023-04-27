void ff_vdpau_h264_picture_complete(H264Context *h)
{
    struct vdpau_render_state *render;
    render = (struct vdpau_render_state *)h->cur_pic_ptr->f.data[0];
    assert(render);
    render->info.h264.slice_count = h->slice_num;
    if (render->info.h264.slice_count < 1)
        return;
    render->info.h264.is_reference = (h->cur_pic_ptr->f.reference & 3) ? VDP_TRUE : VDP_FALSE;
    render->info.h264.field_pic_flag = h->picture_structure != PICT_FRAME;
    render->info.h264.bottom_field_flag = h->picture_structure == PICT_BOTTOM_FIELD;
    render->info.h264.num_ref_frames = h->sps.ref_frame_count;
    render->info.h264.mb_adaptive_frame_field_flag = h->sps.mb_aff && !render->info.h264.field_pic_flag;
    render->info.h264.constrained_intra_pred_flag = h->pps.constrained_intra_pred;
    render->info.h264.weighted_pred_flag = h->pps.weighted_pred;
    render->info.h264.weighted_bipred_idc = h->pps.weighted_bipred_idc;
    render->info.h264.frame_mbs_only_flag = h->sps.frame_mbs_only_flag;
    render->info.h264.transform_8x8_mode_flag = h->pps.transform_8x8_mode;
    render->info.h264.chroma_qp_index_offset = h->pps.chroma_qp_index_offset[0];
    render->info.h264.second_chroma_qp_index_offset = h->pps.chroma_qp_index_offset[1];
    render->info.h264.pic_init_qp_minus26 = h->pps.init_qp - 26;
    render->info.h264.num_ref_idx_l0_active_minus1 = h->pps.ref_count[0] - 1;
    render->info.h264.num_ref_idx_l1_active_minus1 = h->pps.ref_count[1] - 1;
    render->info.h264.log2_max_frame_num_minus4 = h->sps.log2_max_frame_num - 4;
    render->info.h264.pic_order_cnt_type = h->sps.poc_type;
    render->info.h264.log2_max_pic_order_cnt_lsb_minus4 = h->sps.poc_type ? 0 : h->sps.log2_max_poc_lsb - 4;
    render->info.h264.delta_pic_order_always_zero_flag = h->sps.delta_pic_order_always_zero_flag;
    render->info.h264.direct_8x8_inference_flag = h->sps.direct_8x8_inference_flag;
    render->info.h264.entropy_coding_mode_flag = h->pps.cabac;
    render->info.h264.pic_order_present_flag = h->pps.pic_order_present;
    render->info.h264.deblocking_filter_control_present_flag = h->pps.deblocking_filter_parameters_present;
    render->info.h264.redundant_pic_cnt_present_flag = h->pps.redundant_pic_cnt_present;
    memcpy(render->info.h264.scaling_lists_4x4, h->pps.scaling_matrix4, sizeof(render->info.h264.scaling_lists_4x4));
    memcpy(render->info.h264.scaling_lists_8x8[0], h->pps.scaling_matrix8[0], sizeof(render->info.h264.scaling_lists_8x8[0]));
    memcpy(render->info.h264.scaling_lists_8x8[1], h->pps.scaling_matrix8[3], sizeof(render->info.h264.scaling_lists_8x8[0]));
    ff_h264_draw_horiz_band(h, 0, h->avctx->height);
    render->bitstream_buffers_used = 0;
}