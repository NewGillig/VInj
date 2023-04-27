static void evhttp_make_header_request(struct evhttp_connection *evcon, struct evhttp_request *req)
{
    const char *method;
    evhttp_remove_header(req->output_headers, "Proxy-Connection");
    method = evhttp_method(req->type);
    evbuffer_add_printf(evcon->output_buffer, "%s %s HTTP/%d.%d\r\n", method, req->uri, req->major, req->minor);
    if (req->type == EVHTTP_REQ_POST && evhttp_find_header(req->output_headers, "Content-Length") == NULL)
    {
        char size[22];
        evutil_snprintf(size, sizeof(size), "%ld", (long)EVBUFFER_LENGTH(req->output_buffer));
        evhttp_add_header(req->output_headers, "Content-Length", size);
    }
}