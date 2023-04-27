enum message_cte message_decoder_parse_cte(struct message_header_line *hdr)
{
    struct rfc822_parser_context parser;
    enum message_cte message_cte;
    string_t *value;
    value = t_str_new(64);
    rfc822_parser_init(&parser, hdr->full_value, hdr->full_value_len, NULL);
    rfc822_skip_lwsp(&parser);
    (void)rfc822_parse_mime_token(&parser, value);
    message_cte = MESSAGE_CTE_UNKNOWN;
    switch (str_len(value))
    {
    case 4:
        if (i_memcasecmp(str_data(value), "7bit", 4) == 0 || i_memcasecmp(str_data(value), "8bit", 4) == 0)
            message_cte = MESSAGE_CTE_78BIT;
        break;
    case 6:
        if (i_memcasecmp(str_data(value), "base64", 6) == 0)
            message_cte = MESSAGE_CTE_BASE64;
        else if (i_memcasecmp(str_data(value), "binary", 6) == 0)
            message_cte = MESSAGE_CTE_BINARY;
        break;
    case 16:
        if (i_memcasecmp(str_data(value), "quoted-printable", 16) == 0)
            message_cte = MESSAGE_CTE_QP;
        break;
    }
    return message_cte;
}