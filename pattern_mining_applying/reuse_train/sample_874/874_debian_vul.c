int archive_read_support_format_all(struct archive *a)
{
    archive_check_magic(a, ARCHIVE_READ_MAGIC, ARCHIVE_STATE_NEW, "archive_read_support_format_all");
    archive_read_support_format_ar(a);
    archive_read_support_format_cpio(a);
    archive_read_support_format_empty(a);
    archive_read_support_format_lha(a);
    archive_read_support_format_mtree(a);
    archive_read_support_format_tar(a);
    archive_read_support_format_xar(a);
    archive_read_support_format_7zip(a);
    archive_read_support_format_cab(a);
    archive_read_support_format_rar(a);
    archive_read_support_format_iso9660(a);
    archive_read_support_format_zip(a);
    archive_clear_error(a);
    return (ARCHIVE_OK);
}