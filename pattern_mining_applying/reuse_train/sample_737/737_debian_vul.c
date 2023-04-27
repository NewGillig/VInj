static void purple_logout(struct im_connection *ic)
{
    struct purple_data *pd = ic->proto_data;
    if (!pd)
    {
        return;
    }
    while (ic->groupchats)
    {
        imcb_chat_free(ic->groupchats->data);
    }
    purple_account_set_enabled(pd->account, "BitlBee", FALSE);
    purple_connections = g_slist_remove(purple_connections, ic);
    purple_accounts_remove(pd->account);
    imcb_chat_list_free(ic);
    g_free(pd->chat_list_server);
    g_hash_table_destroy(pd->input_requests);
    g_free(pd);
}