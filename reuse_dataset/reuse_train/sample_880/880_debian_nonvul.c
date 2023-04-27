static char **create_argv_command(struct rule *rule, struct process *process, struct iovec **argv)
{
    size_t count, i, j, stdin_arg;
    char **req_argv = NULL;
    const char *program;
    for (count = 0; argv[count] != NULL; count++)
        ;
    if (rule->sudo_user == NULL)
        req_argv = xcalloc(count + 1, sizeof(char *));
    else
        req_argv = xcalloc(count + 5, sizeof(char *));
    if (rule->sudo_user != NULL)
    {
        req_argv[0] = xstrdup(PATH_SUDO);
        req_argv[1] = xstrdup("-u");
        req_argv[2] = xstrdup(rule->sudo_user);
        req_argv[3] = xstrdup("--");
        req_argv[4] = xstrdup(rule->program);
        j = 5;
    }
    else
    {
        program = strrchr(rule->program, '/');
        if (program == NULL)
            program = rule->program;
        else
            program++;
        req_argv[0] = xstrdup(program);
        j = 1;
    }
    if (rule->stdin_arg == -1)
        stdin_arg = count - 1;
    else
        stdin_arg = (size_t)rule->stdin_arg;
    for (i = 1; i < count; i++)
    {
        const char *data = argv[i]->iov_base;
        size_t length = argv[i]->iov_len;
        if (i == stdin_arg)
        {
            process->input = evbuffer_new();
            if (process->input == NULL)
                die("internal error: cannot create input buffer");
            if (evbuffer_add(process->input, data, length) < 0)
                die("internal error: cannot add data to input buffer");
            continue;
        }
        if (length == 0)
            req_argv[j] = xstrdup("");
        else
            req_argv[j] = xstrndup(data, length);
        j++;
    }
    req_argv[j] = NULL;
    return req_argv;
}