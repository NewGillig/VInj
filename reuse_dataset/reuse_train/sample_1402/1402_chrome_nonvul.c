static void optimize_b(MACROBLOCK *mb, int ib, int type, ENTROPY_CONTEXT *a, ENTROPY_CONTEXT *l)
{
    BLOCK *b;
    BLOCKD *d;
    vp8_token_state tokens[17][2];
    unsigned best_mask[2];
    const short *dequant_ptr;
    const short *coeff_ptr;
    short *qcoeff_ptr;
    short *dqcoeff_ptr;
    int eob;
    int i0;
    int rc;
    int x;
    int sz = 0;
    int next;
    int rdmult;
    int rddiv;
    int final_eob;
    int rd_cost0;
    int rd_cost1;
    int rate0;
    int rate1;
    int error0;
    int error1;
    int t0;
    int t1;
    int best;
    int band;
    int pt;
    int i;
    int err_mult = plane_rd_mult[type];
    b = &mb->block[ib];
    d = &mb->e_mbd.block[ib];
    dequant_ptr = d->dequant;
    coeff_ptr = b->coeff;
    qcoeff_ptr = d->qcoeff;
    dqcoeff_ptr = d->dqcoeff;
    i0 = !type;
    eob = *d->eob;
    rdmult = mb->rdmult * err_mult;
    if (mb->e_mbd.mode_info_context->mbmi.ref_frame == INTRA_FRAME)
        rdmult = (rdmult * 9) >> 4;
    rddiv = mb->rddiv;
    best_mask[0] = best_mask[1] = 0;
    tokens[eob][0].rate = 0;
    tokens[eob][0].error = 0;
    tokens[eob][0].next = 16;
    tokens[eob][0].token = DCT_EOB_TOKEN;
    tokens[eob][0].qc = 0;
    *(tokens[eob] + 1) = *(tokens[eob] + 0);
    next = eob;
    for (i = eob; i-- > i0;)
    {
        int base_bits;
        int d2;
        int dx;
        rc = vp8_default_zig_zag1d[i];
        x = qcoeff_ptr[rc];
        if (x)
        {
            int shortcut = 0;
            error0 = tokens[next][0].error;
            error1 = tokens[next][1].error;
            rate0 = tokens[next][0].rate;
            rate1 = tokens[next][1].rate;
            t0 = (vp8_dct_value_tokens_ptr + x)->Token;
            if (next < 16)
            {
                band = vp8_coef_bands[i + 1];
                pt = vp8_prev_token_class[t0];
                rate0 += mb->token_costs[type][band][pt][tokens[next][0].token];
                rate1 += mb->token_costs[type][band][pt][tokens[next][1].token];
            }
            rd_cost0 = RDCOST(rdmult, rddiv, rate0, error0);
            rd_cost1 = RDCOST(rdmult, rddiv, rate1, error1);
            if (rd_cost0 == rd_cost1)
            {
                rd_cost0 = RDTRUNC(rdmult, rddiv, rate0, error0);
                rd_cost1 = RDTRUNC(rdmult, rddiv, rate1, error1);
            }
            best = rd_cost1 < rd_cost0;
            base_bits = *(vp8_dct_value_cost_ptr + x);
            dx = dqcoeff_ptr[rc] - coeff_ptr[rc];
            d2 = dx * dx;
            tokens[i][0].rate = base_bits + (best ? rate1 : rate0);
            tokens[i][0].error = d2 + (best ? error1 : error0);
            tokens[i][0].next = next;
            tokens[i][0].token = t0;
            tokens[i][0].qc = x;
            best_mask[0] |= best << i;
            rate0 = tokens[next][0].rate;
            rate1 = tokens[next][1].rate;
            if ((abs(x) * dequant_ptr[rc] > abs(coeff_ptr[rc])) && (abs(x) * dequant_ptr[rc] < abs(coeff_ptr[rc]) + dequant_ptr[rc]))
                shortcut = 1;
            else
                shortcut = 0;
            if (shortcut)
            {
                sz = -(x < 0);
                x -= 2 * sz + 1;
            }
            if (!x)
            {
                t0 = tokens[next][0].token == DCT_EOB_TOKEN ? DCT_EOB_TOKEN : ZERO_TOKEN;
                t1 = tokens[next][1].token == DCT_EOB_TOKEN ? DCT_EOB_TOKEN : ZERO_TOKEN;
            }
            else
            {
                t0 = t1 = (vp8_dct_value_tokens_ptr + x)->Token;
            }
            if (next < 16)
            {
                band = vp8_coef_bands[i + 1];
                if (t0 != DCT_EOB_TOKEN)
                {
                    pt = vp8_prev_token_class[t0];
                    rate0 += mb->token_costs[type][band][pt][tokens[next][0].token];
                }
                if (t1 != DCT_EOB_TOKEN)
                {
                    pt = vp8_prev_token_class[t1];
                    rate1 += mb->token_costs[type][band][pt][tokens[next][1].token];
                }
            }
            rd_cost0 = RDCOST(rdmult, rddiv, rate0, error0);
            rd_cost1 = RDCOST(rdmult, rddiv, rate1, error1);
            if (rd_cost0 == rd_cost1)
            {
                rd_cost0 = RDTRUNC(rdmult, rddiv, rate0, error0);
                rd_cost1 = RDTRUNC(rdmult, rddiv, rate1, error1);
            }
            best = rd_cost1 < rd_cost0;
            base_bits = *(vp8_dct_value_cost_ptr + x);
            if (shortcut)
            {
                dx -= (dequant_ptr[rc] + sz) ^ sz;
                d2 = dx * dx;
            }
            tokens[i][1].rate = base_bits + (best ? rate1 : rate0);
            tokens[i][1].error = d2 + (best ? error1 : error0);
            tokens[i][1].next = next;
            tokens[i][1].token = best ? t1 : t0;
            tokens[i][1].qc = x;
            best_mask[1] |= best << i;
            next = i;
        }
        else
        {
            band = vp8_coef_bands[i + 1];
            t0 = tokens[next][0].token;
            t1 = tokens[next][1].token;
            if (t0 != DCT_EOB_TOKEN)
            {
                tokens[next][0].rate += mb->token_costs[type][band][0][t0];
                tokens[next][0].token = ZERO_TOKEN;
            }
            if (t1 != DCT_EOB_TOKEN)
            {
                tokens[next][1].rate += mb->token_costs[type][band][0][t1];
                tokens[next][1].token = ZERO_TOKEN;
            }
        }
    }
    band = vp8_coef_bands[i + 1];
    VP8_COMBINEENTROPYCONTEXTS(pt, *a, *l);
    rate0 = tokens[next][0].rate;
    rate1 = tokens[next][1].rate;
    error0 = tokens[next][0].error;
    error1 = tokens[next][1].error;
    t0 = tokens[next][0].token;
    t1 = tokens[next][1].token;
    rate0 += mb->token_costs[type][band][pt][t0];
    rate1 += mb->token_costs[type][band][pt][t1];
    rd_cost0 = RDCOST(rdmult, rddiv, rate0, error0);
    rd_cost1 = RDCOST(rdmult, rddiv, rate1, error1);
    if (rd_cost0 == rd_cost1)
    {
        rd_cost0 = RDTRUNC(rdmult, rddiv, rate0, error0);
        rd_cost1 = RDTRUNC(rdmult, rddiv, rate1, error1);
    }
    best = rd_cost1 < rd_cost0;
    final_eob = i0 - 1;
    for (i = next; i < eob; i = next)
    {
        x = tokens[i][best].qc;
        if (x)
            final_eob = i;
        rc = vp8_default_zig_zag1d[i];
        qcoeff_ptr[rc] = x;
        dqcoeff_ptr[rc] = x * dequant_ptr[rc];
        next = tokens[i][best].next;
        best = (best_mask[best] >> i) & 1;
    }
    final_eob++;
    *a = *l = (final_eob != !type);
    *d->eob = (char)final_eob;
}