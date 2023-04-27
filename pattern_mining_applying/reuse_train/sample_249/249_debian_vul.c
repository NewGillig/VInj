static void test_rfc822_parse_quoted_string(void)
{
    const struct
    {
        const char *input, *output;
        int ret;
    } tests[] = {{"\"", "", -1}, {"\"\"", "", 0}, {"\"foo\"", "foo", 0}, {"\"\"foo", "", 1}, {"\"\"\"", "", 1}, {"\"\\\"\"", "\"", 0}, {"\"\\\\\"", "\\", 0}, {"\"\\\\foo\\\\foo\\\\\"", "\\foo\\foo\\", 0}};
    struct rfc822_parser_context parser;
    string_t *str = t_str_new(64);
    unsigned int i = 0;
    test_begin("rfc822 parse quoted string");
    for (i = 0; i < N_ELEMENTS(tests); i++)
    {
        rfc822_parser_init(&parser, (const void *)tests[i].input, strlen(tests[i].input), NULL);
        test_assert_idx(rfc822_parse_quoted_string(&parser, str) == tests[i].ret, i);
        test_assert_idx(tests[i].ret < 0 || strcmp(tests[i].output, str_c(str)) == 0, i);
        str_truncate(str, 0);
    }
    test_end();
}