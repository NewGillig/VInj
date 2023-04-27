static int do_cmd(xd3_stream *stream, const char *buf)
{
    int ret;
    if ((ret = system(buf)) != 0)
    {
        if (WIFEXITED(ret))
        {
            stream->msg = "command exited non-zero";
            IF_DEBUG1(XPR(NT "command was: %s\n", buf));
        }
        else
        {
            stream->msg = "abnormal command termination";
        }
        return XD3_INTERNAL;
    }
    return 0;
}