void vp9_encode_intra_block_plane(MACROBLOCK *x, BLOCK_SIZE bsize, int plane)
{
    const MACROBLOCKD *const xd = &x->e_mbd;
    struct encode_b_args arg = {x, NULL, &xd->mi[0].src_mi->mbmi.skip};
    vp9_foreach_transformed_block_in_plane(xd, bsize, plane, encode_block_intra, &arg);
}