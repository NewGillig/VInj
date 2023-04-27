ParseResult mime_parser_parse(MIMEParser *parser, HdrHeap *heap, MIMEHdrImpl *mh, const char **real_s, const char *real_e, bool must_copy_strings, bool eof)
{
    ParseResult err;
    bool line_is_real;
    const char *colon;
    const char *line_c;
    const char *line_s = nullptr;
    const char *line_e = nullptr;
    const char *field_name_first;
    const char *field_name_last;
    const char *field_value_first;
    const char *field_value_last;
    const char *field_line_first;
    const char *field_line_last;
    int field_name_length, field_value_length;
    MIMEScanner *scanner = &parser->m_scanner;
    while (true)
    {
        err = mime_scanner_get(scanner, real_s, real_e, &line_s, &line_e, &line_is_real, eof, MIME_SCANNER_TYPE_FIELD);
        if (err != PARSE_RESULT_OK)
        {
            return err;
        }
        line_c = line_s;
        if ((line_e - line_c >= 2) && (line_c[0] == ParseRules
                                       :
                                       : CHAR_CR) &&
            (line_c[1] == ParseRules
             :
             : CHAR_LF))
        {
            return PARSE_RESULT_DONE;
        }
        if ((line_e - line_c >= 1) && (line_c[0] == ParseRules
                                       :
                                       : CHAR_LF))
        {
            return PARSE_RESULT_DONE;
        }
        field_line_first = line_c;
        field_line_last = line_e - 1;
        field_name_first = line_c;
        if ((!ParseRules
             :
             : is_token(*field_name_first)) &&
            (*field_name_first != '@'))
        {
            continue;
        }
        colon = (char *)memchr(line_c, ':', (line_e - line_c));
        if (!colon)
        {
            continue;
        }
        field_name_last = colon - 1;
        if ((field_name_last >= field_name_first) && is_ws(*field_name_last))
        {
            return PARSE_RESULT_ERROR;
        }
        field_value_first = colon + 1;
        while ((field_value_first < line_e) && is_ws(*field_value_first))
        {
            ++field_value_first;
        }
        field_value_last = line_e - 1;
        while ((field_value_last >= field_value_first) && ParseRules
               :
               : is_wslfcr(*field_value_last))
        {
            --field_value_last;
        }
        field_name_length = (int)(field_name_last - field_name_first + 1);
        field_value_length = (int)(field_value_last - field_value_first + 1);
        if (field_name_length >= UINT16_MAX || field_value_length >= UINT16_MAX)
        {
            return PARSE_RESULT_ERROR;
        }
        int total_line_length = (int)(field_line_last - field_line_first + 1);
        if (must_copy_strings || (!line_is_real))
        {
            int length = total_line_length;
            char *dup = heap->duplicate_str(field_name_first, length);
            intptr_t delta = dup - field_name_first;
            field_name_first += delta;
            field_value_first += delta;
        }
        int field_name_wks_idx = hdrtoken_tokenize(field_name_first, field_name_length);
        MIMEField *field = mime_field_create(heap, mh);
        mime_field_name_value_set(heap, mh, field, field_name_wks_idx, field_name_first, field_name_length, field_value_first, field_value_length, true, total_line_length, false);
        mime_hdr_field_attach(mh, field, 1, nullptr);
    }
}