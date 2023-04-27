int main(int argc, char **argv)
{
    char inputbuf[4096];
    unsigned char *buf;
    size_t len;
    gpg_error_t err;
    if (argc == 2 && !strcmp(argv[1], "--to-str"))
    {
        fread(inputbuf, 1, sizeof inputbuf, stdin);
        if (!feof(stdin))
            fail("read error or input too large");
        fail("not yet implemented");
    }
    else if (argc == 2 && !strcmp(argv[1], "--to-der"))
    {
        fread(inputbuf, 1, sizeof inputbuf, stdin);
        if (!feof(stdin))
            fail("read error or input too large");
        err = ksba_dn_str2der(inputbuf, &buf, &len);
        fail_if_err(err);
        fwrite(buf, len, 1, stdout);
    }
    else if (argc == 1)
    {
        test_0();
        test_1();
        test_2();
    }
    else
    {
        fprintf(stderr, "usage: t-dnparser [--to-str|--to-der]\n");
        return 1;
    }
    return 0;
}