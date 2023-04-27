int encode_const(char *string, struct filter_op *fop)
{
    char *p;
    memset(fop, 0, sizeof(struct filter_op));
    if (!strncmp(string, "0x", 2) && isxdigit((int)string[2]))
    {
        fop->op.test.value = strtoul(string, NULL, 16);
        return E_SUCCESS;
    }
    else if (isdigit((int)string[0]))
    {
        fop->op.test.value = strtoul(string, NULL, 10);
        return E_SUCCESS;
    }
    else if (string[0] == '\'' && string[strlen(string) - 1] == '\'')
    {
        struct ip_addr ipaddr;
        p = strchr(string + 1, '\'');
        *p = '\0';
        if (ip_addr_pton(string + 1, &ipaddr) == E_SUCCESS)
        {
            switch (ntohs(ipaddr.addr_type))
            {
            case AF_INET:
                fop->op.test.value = ntohl(ipaddr.addr32[0]);
                break;
            case AF_INET6:
                ip_addr_cpy((u_char *)&fop->op.test.ipaddr, &ipaddr);
                break;
            default:
                return -E_FATAL;
            }
        }
        else
        {
            return -E_FATAL;
        }
        return E_SUCCESS;
    }
    else if (string[0] == '\"' && string[strlen(string) - 1] == '\"')
    {
        p = strchr(string + 1, '\"');
        *p = '\0';
        fop->op.test.string = (u_char *)strdup(string + 1);
        fop->op.test.slen = strescape((char *)fop->op.test.string, (char *)fop->op.test.string, strlen(fop->op.test.string) + 1);
        return E_SUCCESS;
    }
    else if (isalpha((int)string[0]))
    {
        return get_constant(string, &fop->op.test.value);
    }
    return -E_NOTFOUND;
}