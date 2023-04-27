static void config_connection_input(struct config_connection *conn)
{
    const char *const *args, *line;
    switch (i_stream_read(conn->input))
    {
    case -2:
        i_error("BUG: Config client connection sent too much data");
        config_connection_destroy(conn);
        return;
    case -1:
        config_connection_destroy(conn);
        return;
    }
    if (!conn->version_received)
    {
        line = i_stream_next_line(conn->input);
        if (line == NULL)
            return;
        if (!version_string_verify(line, "config", CONFIG_CLIENT_PROTOCOL_MAJOR_VERSION))
        {
            i_error("Config client not compatible with this server "
                    "(mixed old and new binaries?)");
            config_connection_destroy(conn);
            return;
        }
        conn->version_received = TRUE;
    }
    while ((args = config_connection_next_line(conn)) != NULL)
    {
        if (args[0] == NULL)
            continue;
        if (strcmp(args[0], "REQ") == 0)
        {
            if (config_connection_request(conn, args + 1) < 0)
                break;
        }
        if (strcmp(args[0], "FILTERS") == 0)
        {
            if (config_filters_request(conn) < 0)
                break;
        }
    }
}