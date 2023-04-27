static void handle_raw_data(char *packet, int len, struct query *q, int dns_fd, int tun_fd, int userid)
{
    if (check_authenticated_user_and_ip(userid, q) != 0)
    {
        return;
    }
    if (!users[userid].authenticated_raw)
        return;
    users[userid].last_pkt = time(NULL);
    memcpy(&(users[userid].q), q, sizeof(struct query));
    users[userid].inpacket.offset = 0;
    memcpy(users[userid].inpacket.data, packet, len);
    users[userid].inpacket.len = len;
    if (debug >= 1)
    {
        fprintf(stderr, "IN pkt raw, total %d, from user %d\n", users[userid].inpacket.len, userid);
    }
    handle_full_packet(tun_fd, dns_fd, userid);
}