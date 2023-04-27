int jbig2_decode_halftone_region(Jbig2Ctx *ctx, Jbig2Segment *segment, Jbig2HalftoneRegionParams *params, const byte *data, const size_t size, Jbig2Image *image, Jbig2ArithCx *GB_stats)
{
    uint32_t HBPP;
    uint32_t HNUMPATS;
    uint8_t **GI;
    Jbig2Image *HSKIP = NULL;
    Jbig2PatternDict *HPATS;
    int i;
    uint32_t mg, ng;
    int32_t x, y;
    uint8_t gray_val;
    memset(image->data, params->HDEFPIXEL, image->stride * image->height);
    if (params->HENABLESKIP == 1)
    {
        jbig2_error(ctx, JBIG2_SEVERITY_WARNING, segment->number, "unhandled option HENABLESKIP");
    }
    HPATS = jbig2_decode_ht_region_get_hpats(ctx, segment);
    if (!HPATS)
    {
        jbig2_error(ctx, JBIG2_SEVERITY_WARNING, segment->number, "no pattern dictionary found, skipping halftone image");
        return -1;
    }
    HNUMPATS = HPATS->n_patterns;
    HBPP = 0;
    while (HNUMPATS > (1 << ++HBPP))
        ;
    GI = jbig2_decode_gray_scale_image(ctx, segment, data, size, params->HMMR, params->HGW, params->HGH, HBPP, params->HENABLESKIP, HSKIP, params->HTEMPLATE, GB_stats);
    if (!GI)
    {
        jbig2_error(ctx, JBIG2_SEVERITY_WARNING, segment->number, "unable to acquire gray-scale image, skipping halftone image");
        return -1;
    }
    for (mg = 0; mg < params->HGH; ++mg)
    {
        for (ng = 0; ng < params->HGW; ++ng)
        {
            x = (params->HGX + mg * params->HRY + ng * params->HRX) >> 8;
            y = (params->HGY + mg * params->HRX - ng * params->HRY) >> 8;
            gray_val = GI[ng][mg];
            if (gray_val >= HNUMPATS)
            {
                jbig2_error(ctx, JBIG2_SEVERITY_WARNING, segment->number, "gray-scale image uses value %d which larger than pattern dictionary", gray_val);
                gray_val = HNUMPATS - 1;
            }
            jbig2_image_compose(ctx, image, HPATS->patterns[gray_val], x, y, params->op);
        }
    }
    for (i = 0; i < params->HGW; ++i)
    {
        jbig2_free(ctx->allocator, GI[i]);
    }
    jbig2_free(ctx->allocator, GI);
    return 0;
}