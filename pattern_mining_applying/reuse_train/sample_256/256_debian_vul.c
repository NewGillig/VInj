static void parse_content_type(struct message_search_context *ctx, struct message_header_line *hdr)
{
    struct rfc822_parser_context parser;
    string_t *content_type;
    rfc822_parser_init(&parser, hdr->full_value, hdr->full_value_len, NULL);
    rfc822_skip_lwsp(&parser);
    content_type = t_str_new(64);
    (void)rfc822_parse_content_type(&parser, content_type);
    ctx->content_type_text = strncasecmp(str_c(content_type), "text/", 5) == 0 || strncasecmp(str_c(content_type), "message/", 8) == 0;
}