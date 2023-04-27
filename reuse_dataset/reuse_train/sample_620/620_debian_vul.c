static int test_save_copy(const char *origname)
{
    char buf[TESTBUFSIZE];
    int ret;
    snprintf_func(buf, TESTBUFSIZE, "cp -f %s %s", origname, TEST_COPY_FILE);
    if ((ret = system(buf)) != 0)
    {
        return XD3_INTERNAL;
    }
    return 0;
}