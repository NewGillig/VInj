int vp9_get_tx_size_context(const MACROBLOCKD *xd)
{
    const int max_tx_size = max_txsize_lookup[xd->mi[0]->mbmi.sb_type];
    const MB_MODE_INFO *const above_mbmi = get_mbmi(get_above_mi(xd));
    const MB_MODE_INFO *const left_mbmi = get_mbmi(get_left_mi(xd));
    const int has_above = above_mbmi != NULL;
    const int has_left = left_mbmi != NULL;
    int above_ctx = (has_above && !above_mbmi->skip) ? (int)above_mbmi->tx_size : max_tx_size;
    int left_ctx = (has_left && !left_mbmi->skip) ? (int)left_mbmi->tx_size : max_tx_size;
    if (!has_left)
        left_ctx = above_ctx;
    if (!has_above)
        above_ctx = left_ctx;
    return (above_ctx + left_ctx) > max_tx_size;
}