static enum nss_status getanswer_r(const querybuf *answer, int anslen, const char *qname, int qtype, struct hostent *result, char *buffer, size_t buflen, int *errnop, int *h_errnop, int map, int32_t *ttlp, char **canonp)
{
    struct host_data
    {
        char *aliases[MAX_NR_ALIASES];
        unsigned char host_addr[16];
        char *h_addr_ptrs[0];
    } * host_data;
    int linebuflen;
    const HEADER *hp;
    const u_char *end_of_message, *cp;
    int n, ancount, qdcount;
    int haveanswer, had_error;
    char *bp, **ap, **hap;
    char tbuf[MAXDNAME];
    const char *tname;
    int (*name_ok)(const char *);
    u_char packtmp[NS_MAXCDNAME];
    int have_to_map = 0;
    uintptr_t pad = -(uintptr_t)buffer % __alignof__(struct host_data);
    buffer += pad;
    if (__glibc_unlikely(buflen < sizeof(struct host_data) + pad))
    {
    too_small:
        *errnop = ERANGE;
        *h_errnop = NETDB_INTERNAL;
        return NSS_STATUS_TRYAGAIN;
    }
    host_data = (struct host_data *)buffer;
    linebuflen = buflen - sizeof(struct host_data);
    if (buflen - sizeof(struct host_data) != linebuflen)
        linebuflen = INT_MAX;
    tname = qname;
    result->h_name = NULL;
    end_of_message = answer->buf + anslen;
    switch (qtype)
    {
    case T_A:
    case T_AAAA:
        name_ok = res_hnok;
        break;
    case T_PTR:
        name_ok = res_dnok;
        break;
    default:
        *errnop = ENOENT;
        return NSS_STATUS_UNAVAIL;
    }
    hp = &answer->hdr;
    ancount = ntohs(hp->ancount);
    qdcount = ntohs(hp->qdcount);
    cp = answer->buf + HFIXEDSZ;
    if (__builtin_expect(qdcount, 1) != 1)
    {
        *h_errnop = NO_RECOVERY;
        return NSS_STATUS_UNAVAIL;
    }
    if (sizeof(struct host_data) + (ancount + 1) * sizeof(char *) >= buflen)
        goto too_small;
    bp = (char *)&host_data->h_addr_ptrs[ancount + 1];
    linebuflen -= (ancount + 1) * sizeof(char *);
    n = __ns_name_unpack(answer->buf, end_of_message, cp, packtmp, sizeof packtmp);
    if (n != -1 && __ns_name_ntop(packtmp, bp, linebuflen) == -1)
    {
        if (__builtin_expect(errno, 0) == EMSGSIZE)
            goto too_small;
        n = -1;
    }
    if (n > 0 && bp[0] == '.')
        bp[0] = '\0';
    if (__builtin_expect(n < 0 || ((*name_ok)(bp) == 0 && (errno = EBADMSG)), 0))
    {
        *errnop = errno;
        *h_errnop = NO_RECOVERY;
        return NSS_STATUS_UNAVAIL;
    }
    cp += n + QFIXEDSZ;
    if (qtype == T_A || qtype == T_AAAA)
    {
        n = strlen(bp) + 1;
        if (n >= MAXHOSTNAMELEN)
        {
            *h_errnop = NO_RECOVERY;
            *errnop = ENOENT;
            return NSS_STATUS_TRYAGAIN;
        }
        result->h_name = bp;
        bp += n;
        linebuflen -= n;
        if (linebuflen < 0)
            goto too_small;
        qname = result->h_name;
    }
    ap = host_data->aliases;
    *ap = NULL;
    result->h_aliases = host_data->aliases;
    hap = host_data->h_addr_ptrs;
    *hap = NULL;
    result->h_addr_list = host_data->h_addr_ptrs;
    haveanswer = 0;
    had_error = 0;
    while (ancount-- > 0 && cp < end_of_message && had_error == 0)
    {
        int type, class;
        n = __ns_name_unpack(answer->buf, end_of_message, cp, packtmp, sizeof packtmp);
        if (n != -1 && __ns_name_ntop(packtmp, bp, linebuflen) == -1)
        {
            if (__builtin_expect(errno, 0) == EMSGSIZE)
                goto too_small;
            n = -1;
        }
        if (__glibc_unlikely(n < 0 || (*name_ok)(bp) == 0))
        {
            ++had_error;
            continue;
        }
        cp += n;
        if (__glibc_unlikely(cp + 10 > end_of_message))
        {
            ++had_error;
            continue;
        }
        type = __ns_get16(cp);
        cp += INT16SZ;
        class = __ns_get16(cp);
        cp += INT16SZ;
        int32_t ttl = __ns_get32(cp);
        cp += INT32SZ;
        n = __ns_get16(cp);
        cp += INT16SZ;
        if (__glibc_unlikely(class != C_IN))
        {
            cp += n;
            continue;
        }
        if ((qtype == T_A || qtype == T_AAAA) && type == T_CNAME)
        {
            if (ttlp != NULL && ttl < *ttlp)
                *ttlp = ttl;
            if (ap >= &host_data->aliases[MAX_NR_ALIASES - 1])
                continue;
            n = dn_expand(answer->buf, end_of_message, cp, tbuf, sizeof tbuf);
            if (__glibc_unlikely(n < 0 || (*name_ok)(tbuf) == 0))
            {
                ++had_error;
                continue;
            }
            cp += n;
            *ap++ = bp;
            n = strlen(bp) + 1;
            if (__builtin_expect(n, 0) >= MAXHOSTNAMELEN)
            {
                ++had_error;
                continue;
            }
            bp += n;
            linebuflen -= n;
            n = strlen(tbuf) + 1;
            if (__glibc_unlikely(n > linebuflen))
                goto too_small;
            if (__builtin_expect(n, 0) >= MAXHOSTNAMELEN)
            {
                ++had_error;
                continue;
            }
            result->h_name = bp;
            bp = __mempcpy(bp, tbuf, n);
            linebuflen -= n;
            continue;
        }
        if (qtype == T_PTR && type == T_CNAME)
        {
            n = dn_expand(answer->buf, end_of_message, cp, tbuf, sizeof tbuf);
            if (__glibc_unlikely(n < 0 || res_dnok(tbuf) == 0))
            {
                ++had_error;
                continue;
            }
            cp += n;
            n = strlen(tbuf) + 1;
            if (__glibc_unlikely(n > linebuflen))
                goto too_small;
            if (__builtin_expect(n, 0) >= MAXHOSTNAMELEN)
            {
                ++had_error;
                continue;
            }
            tname = bp;
            bp = __mempcpy(bp, tbuf, n);
            linebuflen -= n;
            continue;
        }
        if (type == T_A && qtype == T_AAAA && map)
            have_to_map = 1;
        else if (__glibc_unlikely(type != qtype))
        {
            if ((_res.options & RES_USE_DNSSEC) == 0)
                syslog(LOG_NOTICE | LOG_AUTH, "gethostby*.getanswer: asked for \"%s %s %s\", "
                                              "got type \"%s\"",
                       qname, p_class(C_IN), p_type(qtype), p_type(type));
            cp += n;
            continue;
        }
        switch (type)
        {
        case T_PTR:
            if (__glibc_unlikely(strcasecmp(tname, bp) != 0))
            {
                syslog(LOG_NOTICE | LOG_AUTH, AskedForGot, qname, bp);
                cp += n;
                continue;
            }
            n = __ns_name_unpack(answer->buf, end_of_message, cp, packtmp, sizeof packtmp);
            if (n != -1 && __ns_name_ntop(packtmp, bp, linebuflen) == -1)
            {
                if (__builtin_expect(errno, 0) == EMSGSIZE)
                    goto too_small;
                n = -1;
            }
            if (__glibc_unlikely(n < 0 || res_hnok(bp) == 0))
            {
                ++had_error;
                break;
            }
            result->h_name = bp;
            if (have_to_map)
            {
                n = strlen(bp) + 1;
                if (__glibc_unlikely(n >= MAXHOSTNAMELEN))
                {
                    ++had_error;
                    break;
                }
                bp += n;
                linebuflen -= n;
                if (map_v4v6_hostent(result, &bp, &linebuflen))
                    goto too_small;
            }
            *h_errnop = NETDB_SUCCESS;
            return NSS_STATUS_SUCCESS;
        case T_A:
        case T_AAAA:
            if (__builtin_expect(strcasecmp(result->h_name, bp), 0) != 0)
            {
                syslog(LOG_NOTICE | LOG_AUTH, AskedForGot, result->h_name, bp);
                cp += n;
                continue;
            }
            if (n != result->h_length)
            {
                cp += n;
                continue;
            }
            if (!haveanswer)
            {
                int nn;
                if (ttlp != NULL && ttl < *ttlp)
                    *ttlp = ttl;
                if (canonp != NULL)
                    *canonp = bp;
                result->h_name = bp;
                nn = strlen(bp) + 1;
                bp += nn;
                linebuflen -= nn;
            }
            linebuflen -= sizeof(align) - ((u_long)bp % sizeof(align));
            bp += sizeof(align) - ((u_long)bp % sizeof(align));
            if (__glibc_unlikely(n > linebuflen))
                goto too_small;
            bp = __mempcpy(*hap++ = bp, cp, n);
            cp += n;
            linebuflen -= n;
            break;
        default:
            abort();
        }
        if (had_error == 0)
            ++haveanswer;
    }
    if (haveanswer > 0)
    {
        *ap = NULL;
        *hap = NULL;
        n = strlen(qname) + 1;
        if (n > linebuflen)
            goto too_small;
        if (n >= MAXHOSTNAMELEN)
            goto no_recovery;
        result->h_name = bp;
        bp = __mempcpy(bp, qname, n);
        linebuflen -= n;
    }
    if (have_to_map)
        if (map_v4v6_hostent(result, &bp, &linebuflen))
            goto too_small;
    *h_errnop = NETDB_SUCCESS;
    return NSS_STATUS_SUCCESS;
}