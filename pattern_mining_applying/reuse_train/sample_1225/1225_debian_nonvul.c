static void sig_message_join(SERVER_REC *server, const char *channel, const char *nick, const char *address)
{
    CHANNEL_REC *chanrec;
    g_return_if_fail(nick != NULL);
    chanrec = channel_find(server, channel);
    if (chanrec != NULL)
        CHANNEL_LAST_MSG_ADD(chanrec, nick, FALSE);
}