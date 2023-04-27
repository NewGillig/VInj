static void fts_build_parse_content_type(struct fts_mail_build_context *ctx, const struct message_header_line *hdr)
{
    struct rfc822_parser_context parser;
    string_t *content_type;
    if (ctx->content_type != NULL)
        return;
    rfc822_parser_init(&parser, hdr->full_value, hdr->full_value_len, NULL);
    rfc822_skip_lwsp(&parser);
    T_BEGIN
    {
        content_type = t_str_new(64);
        (void)rfc822_parse_content_type(&parser, content_type);
        ctx->content_type = str_lcase(i_strdup(str_c(content_type)));
    }
    T_END;
    rfc822_parser_deinit(&parser);
}