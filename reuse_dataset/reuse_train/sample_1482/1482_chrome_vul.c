static vpx_codec_err_t ctrl_set_noise_sensitivity(vpx_codec_alg_priv_t *ctx, va_list args)
{
    struct vp9_extracfg extra_cfg = ctx->extra_cfg;
    extra_cfg.noise_sensitivity = CAST(VP8E_SET_NOISE_SENSITIVITY, args);
    return update_extra_cfg(ctx, &extra_cfg);
}