static int _evdns_nameserver_add_impl(unsigned long int address, int port)
{
    const struct nameserver *server = server_head, *const started_at = server_head;
    struct nameserver *ns;
    int err = 0;
    if (server)
    {
        do
        {
            if (server->address == address)
                return 3;
            server = server->next;
        } while (server != started_at);
    }
    ns = (struct nameserver *)malloc(sizeof(struct nameserver));
    if (!ns)
        return -1;
    memset(ns, 0, sizeof(struct nameserver));
    evtimer_set(&ns->timeout_event, nameserver_prod_callback, ns);
    ns->socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (ns->socket < 0)
    {
        err = 1;
        goto out1;
    }
    FD_CLOSEONEXEC(ns->socket);
    evutil_make_socket_nonblocking(ns->socket);
    ns->address = address;
    ns->port = htons(port);
    ns->state = 1;
    event_set(&ns->event, ns->socket, EV_READ | EV_PERSIST, nameserver_ready_callback, ns);
    if (event_add(&ns->event, NULL) < 0)
    {
        err = 2;
        goto out2;
    }
    log(EVDNS_LOG_DEBUG, "Added nameserver %s", debug_ntoa(address));
    if (!server_head)
    {
        ns->next = ns->prev = ns;
        server_head = ns;
    }
    else
    {
        ns->next = server_head->next;
        ns->prev = server_head;
        server_head->next = ns;
        if (server_head->prev == server_head)
        {
            server_head->prev = ns;
        }
    }
    global_good_nameservers++;
    return 0;
out2:
    CLOSE_SOCKET(ns->socket);
out1:
    free(ns);
    log(EVDNS_LOG_WARN, "Unable to add nameserver %s: error %d", debug_ntoa(address), err);
    return err;
}