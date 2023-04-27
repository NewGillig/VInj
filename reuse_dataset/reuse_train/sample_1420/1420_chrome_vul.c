void vp9_first_pass(VP9_COMP *cpi, const struct lookahead_entry *source)
{
    int mb_row, mb_col;
    MACROBLOCK *const x = &cpi->mb;
    VP9_COMMON *const cm = &cpi->common;
    MACROBLOCKD *const xd = &x->e_mbd;
    TileInfo tile;
    struct macroblock_plane *const p = x->plane;
    struct macroblockd_plane *const pd = xd->plane;
    const PICK_MODE_CONTEXT *ctx = &cpi->pc_root->none;
    int i;
    int recon_yoffset, recon_uvoffset;
    YV12_BUFFER_CONFIG *const lst_yv12 = get_ref_frame_buffer(cpi, LAST_FRAME);
    YV12_BUFFER_CONFIG *gld_yv12 = get_ref_frame_buffer(cpi, GOLDEN_FRAME);
    YV12_BUFFER_CONFIG *const new_yv12 = get_frame_new_buffer(cm);
    int recon_y_stride = lst_yv12->y_stride;
    int recon_uv_stride = lst_yv12->uv_stride;
    int uv_mb_height = 16 >> (lst_yv12->y_height > lst_yv12->uv_height);
    int64_t intra_error = 0;
    int64_t coded_error = 0;
    int64_t sr_coded_error = 0;
    int sum_mvr = 0, sum_mvc = 0;
    int sum_mvr_abs = 0, sum_mvc_abs = 0;
    int64_t sum_mvrs = 0, sum_mvcs = 0;
    int mvcount = 0;
    int intercount = 0;
    int second_ref_count = 0;
    int intrapenalty = 256;
    int neutral_count = 0;
    int new_mv_count = 0;
    int sum_in_vectors = 0;
    MV lastmv = {0, 0};
    TWO_PASS *twopass = &cpi->twopass;
    const MV zero_mv = {0, 0};
    const YV12_BUFFER_CONFIG *first_ref_buf = lst_yv12;
    LAYER_CONTEXT *const lc = is_two_pass_svc(cpi) ? &cpi->svc.layer_context[cpi->svc.spatial_layer_id] : NULL;
    vp9_zero_array(cpi->twopass.frame_mb_stats_buf, cm->MBs);
}