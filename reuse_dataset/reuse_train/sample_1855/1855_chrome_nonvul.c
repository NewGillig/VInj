static struct evhttp *http_setup(short *pport, struct event_base *base)
{
    int i;
    struct evhttp *myhttp;
    short port = -1;
    myhttp = evhttp_new(base);
    for (i = 0; i < 50; ++i)
    {
        if (evhttp_bind_socket(myhttp, "127.0.0.1", 8080 + i) != -1)
        {
            port = 8080 + i;
            break;
        }
    }
    if (port == -1)
        event_errx(1, "Could not start web server");
    evhttp_set_cb(myhttp, "/test", http_basic_cb, NULL);
    evhttp_set_cb(myhttp, "/chunked", http_chunked_cb, NULL);
    evhttp_set_cb(myhttp, "/postit", http_post_cb, NULL);
    evhttp_set_cb(myhttp, "/largedelay", http_large_delay_cb, NULL);
    evhttp_set_cb(myhttp, "/badrequest", http_badreq_cb, NULL);
    evhttp_set_cb(myhttp, "/", http_dispatcher_cb, NULL);
    *pport = port;
    return (myhttp);
}