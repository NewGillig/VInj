static void sig_message_public(SERVER_REC *server, const char *msg, const char *nick, const char *address, const char *target)
{
    CHANNEL_REC *channel;
    int own;
    g_return_if_fail(nick != NULL);
    channel = channel_find(server, target);
    if (channel != NULL)
    {
        own = nick_match_msg(channel, msg, server->nick);
        CHANNEL_LAST_MSG_ADD(channel, nick, own);
    }
}