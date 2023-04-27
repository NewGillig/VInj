static void write_modes_sb(VP9_COMP *cpi, const TileInfo *const tile, vp9_writer *w, TOKENEXTRA **tok, const TOKENEXTRA *const tok_end, int mi_row, int mi_col, BLOCK_SIZE bsize)
{
    const VP9_COMMON *const cm = &cpi->common;
    MACROBLOCKD *const xd = &cpi->mb.e_mbd;
    const int bsl = b_width_log2(bsize);
    const int bs = (1 << bsl) / 4;
    PARTITION_TYPE partition;
    BLOCK_SIZE subsize;
    const MODE_INFO *m = NULL;
    if (mi_row >= cm->mi_rows || mi_col >= cm->mi_cols)
        return;
    m = cm->mi_grid_visible[mi_row * cm->mi_stride + mi_col];
    partition = partition_lookup[bsl][m->mbmi.sb_type];
    write_partition(cm, xd, bs, mi_row, mi_col, partition, bsize, w);
    subsize = get_subsize(bsize, partition);
    if (subsize < BLOCK_8X8)
    {
        write_modes_b(cpi, tile, w, tok, tok_end, mi_row, mi_col);
    }
    else
    {
        switch (partition)
        {
        case PARTITION_NONE:
            write_modes_b(cpi, tile, w, tok, tok_end, mi_row, mi_col);
            break;
        case PARTITION_HORZ:
            write_modes_b(cpi, tile, w, tok, tok_end, mi_row, mi_col);
            if (mi_row + bs < cm->mi_rows)
                write_modes_b(cpi, tile, w, tok, tok_end, mi_row + bs, mi_col);
            break;
        case PARTITION_VERT:
            write_modes_b(cpi, tile, w, tok, tok_end, mi_row, mi_col);
            if (mi_col + bs < cm->mi_cols)
                write_modes_b(cpi, tile, w, tok, tok_end, mi_row, mi_col + bs);
            break;
        case PARTITION_SPLIT:
            write_modes_sb(cpi, tile, w, tok, tok_end, mi_row, mi_col, subsize);
            write_modes_sb(cpi, tile, w, tok, tok_end, mi_row, mi_col + bs, subsize);
            write_modes_sb(cpi, tile, w, tok, tok_end, mi_row + bs, mi_col, subsize);
            write_modes_sb(cpi, tile, w, tok, tok_end, mi_row + bs, mi_col + bs, subsize);
            break;
        default:
            assert(0);
        }
    }
    if (bsize >= BLOCK_8X8 && (bsize == BLOCK_8X8 || partition != PARTITION_SPLIT))
        update_partition_context(xd, mi_row, mi_col, subsize, bsize);
}