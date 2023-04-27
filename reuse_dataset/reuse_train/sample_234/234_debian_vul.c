static void gtkui_inject_user(int side)
{
    size_t len;
    len = strescape(injectbuf, injectbuf);
    if (side == 1 || side == 2)
    {
        user_inject(injectbuf, len, curr_conn, side);
    }
}