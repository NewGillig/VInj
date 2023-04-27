void vp9_foreach_transformed_block_in_plane(const MACROBLOCKD *const xd, BLOCK_SIZE bsize, int plane, foreach_transformed_block_visitor visit, void *arg)
{
    const struct macroblockd_plane *const pd = &xd->plane[plane];
    const MB_MODE_INFO *mbmi = &xd->mi[0].src_mi->mbmi;
    const TX_SIZE tx_size = plane ? get_uv_tx_size(mbmi, pd) : mbmi->tx_size;
    const BLOCK_SIZE plane_bsize = get_plane_block_size(bsize, pd);
    const int num_4x4_w = num_4x4_blocks_wide_lookup[plane_bsize];
    const int num_4x4_h = num_4x4_blocks_high_lookup[plane_bsize];
    const int step = 1 << (tx_size << 1);
    int i;
    if (xd->mb_to_right_edge < 0 || xd->mb_to_bottom_edge < 0)
    {
        int r, c;
        int max_blocks_wide = num_4x4_w;
        int max_blocks_high = num_4x4_h;
        if (xd->mb_to_right_edge < 0)
            max_blocks_wide += (xd->mb_to_right_edge >> (5 + pd->subsampling_x));
        if (xd->mb_to_bottom_edge < 0)
            max_blocks_high += (xd->mb_to_bottom_edge >> (5 + pd->subsampling_y));
        i = 0;
        for (r = 0; r < num_4x4_h; r += (1 << tx_size))
        {
            for (c = 0; c < num_4x4_w; c += (1 << tx_size))
            {
                if (r < max_blocks_high && c < max_blocks_wide)
                    visit(plane, i, plane_bsize, tx_size, arg);
                i += step;
            }
        }
    }
    else
    {
        for (i = 0; i < num_4x4_w * num_4x4_h; i += step)
            visit(plane, i, plane_bsize, tx_size, arg);
    }
}