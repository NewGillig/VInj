static void set_block_size(VP9_COMP *const cpi, int mi_row, int mi_col, BLOCK_SIZE bsize)
{
    if (cpi->common.mi_cols > mi_col && cpi->common.mi_rows > mi_row)
    {
        MACROBLOCKD *const xd = &cpi->mb.e_mbd;
        set_modeinfo_offsets(&cpi->common, xd, mi_row, mi_col);
        xd->mi[0]->mbmi.sb_type = bsize;
        duplicate_mode_info_in_sb(&cpi->common, xd, mi_row, mi_col, bsize);
    }
}