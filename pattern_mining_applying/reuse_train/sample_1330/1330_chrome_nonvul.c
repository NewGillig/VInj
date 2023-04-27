static void print_mi_data(VP9_COMMON *cm, FILE *file, const char *descriptor, size_t member_offset)
{
    int mi_row, mi_col;
    int mi_index = 0;
    MODE_INFO **mi = NULL;
    int rows = cm->mi_rows;
    int cols = cm->mi_cols;
    char prefix = descriptor[0];
    log_frame_info(cm, descriptor, file);
    mi_index = 0;
    for (mi_row = 0; mi_row < rows; mi_row++)
    {
        fprintf(file, "%c ", prefix);
        for (mi_col = 0; mi_col < cols; mi_col++)
        {
            fprintf(file, "%2d ", *((int *)((char *)(&mi[mi_index]->mbmi) + member_offset)));
            mi_index++;
        }
        fprintf(file, "\n");
        mi_index += 8;
    }
    fprintf(file, "\n");
}