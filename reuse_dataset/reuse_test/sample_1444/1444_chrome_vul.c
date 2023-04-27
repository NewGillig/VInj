static void choose_tx_size_from_rd(VP9_COMP *cpi, MACROBLOCK *x, int *rate, int64_t *distortion, int *skip, int64_t *psse, int64_t tx_cache[TX_MODES], int64_t ref_best_rd, BLOCK_SIZE bs)
{
    const TX_SIZE max_tx_size = max_txsize_lookup[bs];
    VP9_COMMON *const cm = &cpi->common;
    MACROBLOCKD *const xd = &x->e_mbd;
    MB_MODE_INFO *const mbmi = &xd->mi[0]->mbmi;
    vp9_prob skip_prob = vp9_get_skip_prob(cm, xd);
    int r[TX_SIZES][2], s[TX_SIZES];
    int64_t d[TX_SIZES], sse[TX_SIZES];
    int64_t rd[TX_SIZES][2] = {{INT64_MAX, INT64_MAX}, {INT64_MAX, INT64_MAX}, {INT64_MAX, INT64_MAX}, {INT64_MAX, INT64_MAX}};
    int n, m;
    int s0, s1;
    const TX_SIZE max_mode_tx_size = tx_mode_to_biggest_tx_size[cm->tx_mode];
    int64_t best_rd = INT64_MAX;
    TX_SIZE best_tx = max_tx_size;
    const vp9_prob *tx_probs = get_tx_probs2(max_tx_size, xd, &cm->fc.tx_probs);
    assert(skip_prob > 0);
    s0 = vp9_cost_bit(skip_prob, 0);
    s1 = vp9_cost_bit(skip_prob, 1);
    for (n = max_tx_size; n >= 0; n--)
    {
        txfm_rd_in_plane(x, &r[n][0], &d[n], &s[n], &sse[n], ref_best_rd, 0, bs, n, cpi->sf.use_fast_coef_costing);
        r[n][1] = r[n][0];
        if (r[n][0] < INT_MAX)
        {
            for (m = 0; m <= n - (n == (int)max_tx_size); m++)
            {
                if (m == n)
                    r[n][1] += vp9_cost_zero(tx_probs[m]);
                else
                    r[n][1] += vp9_cost_one(tx_probs[m]);
            }
        }
        if (d[n] == INT64_MAX)
        {
            rd[n][0] = rd[n][1] = INT64_MAX;
        }
        else if (s[n])
        {
            rd[n][0] = rd[n][1] = RDCOST(x->rdmult, x->rddiv, s1, d[n]);
        }
        else
        {
            rd[n][0] = RDCOST(x->rdmult, x->rddiv, r[n][0] + s0, d[n]);
            rd[n][1] = RDCOST(x->rdmult, x->rddiv, r[n][1] + s0, d[n]);
        }
        if (cpi->sf.tx_size_search_breakout && (rd[n][1] == INT64_MAX || (n < (int)max_tx_size && rd[n][1] > rd[n + 1][1]) || s[n] == 1))
            break;
        if (rd[n][1] < best_rd)
        {
            best_tx = n;
            best_rd = rd[n][1];
        }
    }
    mbmi->tx_size = cm->tx_mode == TX_MODE_SELECT ? best_tx : MIN(max_tx_size, max_mode_tx_size);
    *distortion = d[mbmi->tx_size];
    *rate = r[mbmi->tx_size][cm->tx_mode == TX_MODE_SELECT];
    *skip = s[mbmi->tx_size];
    *psse = sse[mbmi->tx_size];
    tx_cache[ONLY_4X4] = rd[TX_4X4][0];
    tx_cache[ALLOW_8X8] = rd[TX_8X8][0];
    tx_cache[ALLOW_16X16] = rd[MIN(max_tx_size, TX_16X16)][0];
    tx_cache[ALLOW_32X32] = rd[MIN(max_tx_size, TX_32X32)][0];
    if (max_tx_size == TX_32X32 && best_tx == TX_32X32)
    {
        tx_cache[TX_MODE_SELECT] = rd[TX_32X32][1];
    }
    else if (max_tx_size >= TX_16X16 && best_tx == TX_16X16)
    {
        tx_cache[TX_MODE_SELECT] = rd[TX_16X16][1];
    }
    else if (rd[TX_8X8][1] < rd[TX_4X4][1])
    {
        tx_cache[TX_MODE_SELECT] = rd[TX_8X8][1];
    }
    else
    {
        tx_cache[TX_MODE_SELECT] = rd[TX_4X4][1];
    }
}