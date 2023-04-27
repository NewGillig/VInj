void evhttp_read(int fd, short what, void *arg)
{
    struct evhttp_connection *evcon = arg;
    struct evhttp_request *req = TAILQ_FIRST(&evcon->requests);
    struct evbuffer *buf = evcon->input_buffer;
    int n, len;
    if (what == EV_TIMEOUT)
    {
        evhttp_connection_fail(evcon, EVCON_HTTP_TIMEOUT);
        return;
    }
    n = evbuffer_read(buf, fd, -1);
    len = EVBUFFER_LENGTH(buf);
    event_debug(("%s: got %d on %d\n", __func__, n, fd));
    if (n == -1)
    {
        if (errno != EINTR && errno != EAGAIN)
        {
            event_debug(("%s: evbuffer_read", __func__));
            evhttp_connection_fail(evcon, EVCON_HTTP_EOF);
        }
        else
        {
            evhttp_add_event(&evcon->ev, evcon->timeout, HTTP_READ_TIMEOUT);
        }
        return;
    }
    else if (n == 0)
    {
        evcon->state = EVCON_DISCONNECTED;
        evhttp_connection_done(evcon);
        return;
    }
    switch (evcon->state)
    {
    case EVCON_READING_FIRSTLINE:
        evhttp_read_firstline(evcon, req);
        break;
    case EVCON_READING_HEADERS:
        evhttp_read_header(evcon, req);
        break;
    case EVCON_READING_BODY:
        evhttp_read_body(evcon, req);
        break;
    case EVCON_READING_TRAILER:
        evhttp_read_trailer(evcon, req);
        break;
    case EVCON_DISCONNECTED:
    case EVCON_CONNECTING:
    case EVCON_IDLE:
    case EVCON_WRITING:
    default:
        event_errx(1, "%s: illegal connection state %d", __func__, evcon->state);
    }
}