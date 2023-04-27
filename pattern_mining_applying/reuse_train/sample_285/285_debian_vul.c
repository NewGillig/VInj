static int jpc_enc_encodemainbody(jpc_enc_t *enc)
{
    int tileno;
    int tilex;
    int tiley;
    int i;
    jpc_sot_t *sot;
    jpc_enc_tcmpt_t *comp;
    jpc_enc_tcmpt_t *endcomps;
    jpc_enc_band_t *band;
    jpc_enc_band_t *endbands;
    jpc_enc_rlvl_t *lvl;
    int rlvlno;
    jpc_qcc_t *qcc;
    jpc_cod_t *cod;
    int adjust;
    int j;
    int absbandno;
    long numbytes;
    long tilehdrlen;
    long tilelen;
    jpc_enc_tile_t *tile;
    jpc_enc_cp_t *cp;
    double rho;
    int lyrno;
    int cmptno;
    int samestepsizes;
    jpc_enc_ccp_t *ccps;
    jpc_enc_tccp_t *tccp;
    int bandno;
    uint_fast32_t x;
    uint_fast32_t y;
    int mingbits;
    int actualnumbps;
    jpc_fix_t mxmag;
    jpc_fix_t mag;
    int numgbits;
    cp = enc->cp;
    numbytes = 0;
    for (tileno = 0; tileno < JAS_CAST(int, cp->numtiles); ++tileno)
    {
        tilex = tileno % cp->numhtiles;
        tiley = tileno / cp->numhtiles;
        if (!(enc->curtile = jpc_enc_tile_create(enc->cp, enc->image, tileno)))
        {
            abort();
        }
        tile = enc->curtile;
        if (jas_getdbglevel() >= 10)
        {
            jpc_enc_dump(enc);
        }
        endcomps = &tile->tcmpts[tile->numtcmpts];
        for (cmptno = 0, comp = tile->tcmpts; cmptno < tile->numtcmpts; ++cmptno, ++comp)
        {
            if (!cp->ccps[cmptno].sgnd)
            {
                adjust = 1 << (cp->ccps[cmptno].prec - 1);
                for (i = 0; i < jas_matrix_numrows(comp->data); ++i)
                {
                    for (j = 0; j < jas_matrix_numcols(comp->data); ++j)
                    {
                        *jas_matrix_getref(comp->data, i, j) -= adjust;
                    }
                }
            }
        }
        if (!tile->intmode)
        {
            endcomps = &tile->tcmpts[tile->numtcmpts];
            for (comp = tile->tcmpts; comp != endcomps; ++comp)
            {
                jas_matrix_asl(comp->data, JPC_FIX_FRACBITS);
            }
        }
        switch (tile->mctid)
        {
        case JPC_MCT_RCT:
            assert(jas_image_numcmpts(enc->image) == 3);
            jpc_rct(tile->tcmpts[0].data, tile->tcmpts[1].data, tile->tcmpts[2].data);
            break;
        case JPC_MCT_ICT:
            assert(jas_image_numcmpts(enc->image) == 3);
            jpc_ict(tile->tcmpts[0].data, tile->tcmpts[1].data, tile->tcmpts[2].data);
            break;
        default:
            break;
        }
        for (i = 0; i < jas_image_numcmpts(enc->image); ++i)
        {
            comp = &tile->tcmpts[i];
            jpc_tsfb_analyze(comp->tsfb, comp->data);
        }
        endcomps = &tile->tcmpts[tile->numtcmpts];
        for (cmptno = 0, comp = tile->tcmpts; comp != endcomps; ++cmptno, ++comp)
        {
            mingbits = 0;
            absbandno = 0;
            memset(comp->stepsizes, 0, sizeof(comp->stepsizes));
            for (rlvlno = 0, lvl = comp->rlvls; rlvlno < comp->numrlvls; ++rlvlno, ++lvl)
            {
                if (!lvl->bands)
                {
                    absbandno += rlvlno ? 3 : 1;
                    continue;
                }
                endbands = &lvl->bands[lvl->numbands];
                for (band = lvl->bands; band != endbands; ++band)
                {
                    if (!band->data)
                    {
                        ++absbandno;
                        continue;
                    }
                    actualnumbps = 0;
                    mxmag = 0;
                    for (y = 0; y < JAS_CAST(uint_fast32_t, jas_matrix_numrows(band->data)); ++y)
                    {
                        for (x = 0; x < JAS_CAST(uint_fast32_t, jas_matrix_numcols(band->data)); ++x)
                        {
                            mag = JAS_ABS(jas_matrix_get(band->data, y, x));
                            if (mag > mxmag)
                            {
                                mxmag = mag;
                            }
                        }
                    }
                    if (tile->intmode)
                    {
                        actualnumbps = jpc_firstone(mxmag) + 1;
                    }
                    else
                    {
                        actualnumbps = jpc_firstone(mxmag) + 1 - JPC_FIX_FRACBITS;
                    }
                    numgbits = actualnumbps - (cp->ccps[cmptno].prec - 1 + band->analgain);
                    mingbits = numgbits;
                }
                if (!tile->intmode)
                {
                    band->absstepsize = jpc_fix_div(jpc_inttofix(1 << (band->analgain + 1)), band->synweight);
                }
                else
                {
                    band->absstepsize = jpc_inttofix(1);
                }
                band->stepsize = jpc_abstorelstepsize(band->absstepsize, cp->ccps[cmptno].prec + band->analgain);
                band->numbps = cp->tccp.numgbits + JPC_QCX_GETEXPN(band->stepsize) - 1;
                if ((!tile->intmode) && band->data)
                {
                    jpc_quantize(band->data, band->absstepsize);
                }
                comp->stepsizes[absbandno] = band->stepsize;
                ++absbandno;
            }
        }
        assert(JPC_FIX_FRACBITS >= JPC_NUMEXTRABITS);
        if (!tile->intmode)
        {
            jas_matrix_divpow2(comp->data, JPC_FIX_FRACBITS - JPC_NUMEXTRABITS);
        }
        else
        {
            jas_matrix_asl(comp->data, JPC_NUMEXTRABITS);
        }
        jas_eprintf("error: too few guard bits (need at least %d)\n", mingbits);
        return -1;
    }
}