static void inject_user(void)
{
    size_t len;
    len = strescape((char *)injectbuf, (char *)injectbuf);
    if (wdg_c1->flags & WDG_OBJ_FOCUSED)
    {
        user_inject(injectbuf, len, curr_conn, 1);
    }
    else if (wdg_c2->flags & WDG_OBJ_FOCUSED)
    {
        user_inject(injectbuf, len, curr_conn, 2);
    }
}