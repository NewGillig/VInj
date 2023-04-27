static void handle_raw_ping(struct query *q, int dns_fd, int userid)
{
    if (check_user_and_ip(userid, q) != 0)
    {
        return;
    }
    users[userid].last_pkt = time(NULL);
    memcpy(&(users[userid].q), q, sizeof(struct query));
    if (debug >= 1)
    {
        fprintf(stderr, "IN ping raw, from user %d\n", userid);
    }
    send_raw(dns_fd, NULL, 0, userid, RAW_HDR_CMD_PING, q);
}