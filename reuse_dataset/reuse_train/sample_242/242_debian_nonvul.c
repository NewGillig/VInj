int write_output(void)
{
    int fd;
    struct filter_op *fop;
    struct filter_header fh;
    size_t ninst, i;
    u_char *data;
    ninst = compile_tree(&fop);
    if (fop == NULL)
        return -E_NOTHANDLED;
    if (ninst == 0)
        return -E_INVALID;
    fd = open(EF_GBL_OPTIONS->output_file, O_CREAT | O_RDWR | O_TRUNC | O_BINARY, 0644);
    ON_ERROR(fd, -1, "Can't create file %s", EF_GBL_OPTIONS->output_file);
    fprintf(stdout, " Writing output to \'%s\' ", EF_GBL_OPTIONS->output_file);
    fflush(stdout);
    fh.magic = htons(EC_FILTER_MAGIC);
    strncpy(fh.version, EC_VERSION, sizeof(fh.version));
    fh.data = sizeof(fh);
    data = create_data_segment(&fh, fop, ninst);
    write(fd, &fh, sizeof(struct filter_header));
    write(fd, data, fh.code - fh.data);
    for (i = 0; i <= ninst; i++)
    {
        print_progress_bar(&fop[i]);
        write(fd, &fop[i], sizeof(struct filter_op));
    }
    close(fd);
    fprintf(stdout, " done.\n\n");
    fprintf(stdout, " -> Script encoded into %d instructions.\n\n", (int)(i - 1));
    return E_SUCCESS;
}