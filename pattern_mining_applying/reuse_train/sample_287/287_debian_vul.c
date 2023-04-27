struct login_settings *login_settings_read(pool_t pool, const struct ip_addr *local_ip, const struct ip_addr *remote_ip, const char *local_name, const struct master_service_ssl_settings **ssl_set_r, void ***other_settings_r)
{
    struct master_service_settings_input input;
    const char *error;
    const struct setting_parser_context *parser;
    void *const *cache_sets;
    void **sets;
    unsigned int i, count;
    i_zero(&input);
    input.roots = login_set_roots;
    input.module = login_binary->process_name;
    input.service = login_binary->protocol;
    input.local_name = local_name;
    if (local_ip != NULL)
        input.local_ip = *local_ip;
    if (remote_ip != NULL)
        input.remote_ip = *remote_ip;
    if (set_cache == NULL)
    {
        set_cache = master_service_settings_cache_init(master_service, input.module, input.service);
    }
    if (master_service_settings_cache_read(set_cache, &input, NULL, &parser, &error) < 0)
        i_fatal("Error reading configuration: %s", error);
    cache_sets = master_service_settings_parser_get_others(master_service, parser);
    for (count = 0; input.roots[count] != NULL; count++)
        ;
    i_assert(cache_sets[count] == NULL);
    sets = p_new(pool, void *, count + 1);
    for (i = 0; i < count; i++)
        sets[i] = login_setting_dup(pool, input.roots[i], cache_sets[i]);
    settings_var_expand(&login_setting_parser_info, sets[0], pool, login_set_var_expand_table(&input));
    *ssl_set_r = login_setting_dup(pool, &master_service_ssl_setting_parser_info, settings_parser_get_list(parser)[1]);
    *other_settings_r = sets + 1;
    return sets[0];
}