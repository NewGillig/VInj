static int evhttp_parse_request_line(struct evhttp_request *req, char *line)
{
    char *method;
    char *uri;
    char *version;
    method = strsep(&line, " ");
    if (line == NULL)
        return (-1);
    uri = strsep(&line, " ");
    if (line == NULL)
        return (-1);
    version = strsep(&line, " ");
    if (line != NULL)
        return (-1);
    if (strcmp(method, "GET") == 0)
    {
        req->type = EVHTTP_REQ_GET;
    }
    else if (strcmp(method, "POST") == 0)
    {
        req->type = EVHTTP_REQ_POST;
    }
    else if (strcmp(method, "HEAD") == 0)
    {
        req->type = EVHTTP_REQ_HEAD;
    }
    else
    {
        event_debug(("%s: bad method %s on request %p from %s", __func__, method, req, req->remote_host));
        return (-1);
    }
    if (strcmp(version, "HTTP/1.0") == 0)
    {
        req->major = 1;
        req->minor = 0;
    }
    else if (strcmp(version, "HTTP/1.1") == 0)
    {
        req->major = 1;
        req->minor = 1;
    }
    else
    {
        event_debug(("%s: bad version %s on request %p from %s", __func__, version, req, req->remote_host));
        return (-1);
    }
    if ((req->uri = strdup(uri)) == NULL)
    {
        event_debug(("%s: evhttp_decode_uri", __func__));
        return (-1);
    }
    if (strlen(req->uri) > 0 && req->uri[0] != '/')
        req->flags |= EVHTTP_PROXY_REQUEST;
    return (0);
}