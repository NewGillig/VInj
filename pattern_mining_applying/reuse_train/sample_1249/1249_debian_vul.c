static gpgme_error_t status_handler(void *opaque, int fd)
{
    struct io_cb_data *data = (struct io_cb_data *)opaque;
    engine_gpgsm_t gpgsm = (engine_gpgsm_t)data->handler_value;
    gpgme_error_t err = 0;
    char *line;
    size_t linelen;
    do
    {
        err = assuan_read_line(gpgsm->assuan_ctx, &line, &linelen);
        if (err)
        {
            TRACE3(DEBUG_CTX, "gpgme:status_handler", gpgsm, "fd 0x%x: error from assuan (%d) getting status line : %s", fd, err, gpg_strerror(err));
        }
        else if (linelen >= 3 && line[0] == 'E' && line[1] == 'R' && line[2] == 'R' && (line[3] == '\0' || line[3] == ' '))
        {
            if (line[3] == ' ')
                err = atoi(&line[4]);
            if (!err)
                err = gpg_error(GPG_ERR_GENERAL);
            TRACE2(DEBUG_CTX, "gpgme:status_handler", gpgsm, "fd 0x%x: ERR line - mapped to: %s", fd, err ? gpg_strerror(err) : "ok");
        }
        else if (linelen >= 2 && line[0] == 'O' && line[1] == 'K' && (line[2] == '\0' || line[2] == ' '))
        {
            if (gpgsm->status.fnc)
                err = gpgsm->status.fnc(gpgsm->status.fnc_value, GPGME_STATUS_EOF, "");
            if (!err && gpgsm->colon.fnc && gpgsm->colon.any)
            {
                gpgsm->colon.any = 0;
                err = gpgsm->colon.fnc(gpgsm->colon.fnc_value, NULL);
            }
            TRACE2(DEBUG_CTX, "gpgme:status_handler", gpgsm, "fd 0x%x: OK line - final status: %s", fd, err ? gpg_strerror(err) : "ok");
            _gpgme_io_close(gpgsm->status_cb.fd);
            return err;
        }
        else if (linelen > 2 && line[0] == 'D' && line[1] == ' ' && gpgsm->colon.fnc)
        {
            char *src = line + 2;
            char *end = line + linelen;
            char *dst;
            char **aline = &gpgsm->colon.attic.line;
            int *alinelen = &gpgsm->colon.attic.linelen;
            if (gpgsm->colon.attic.linesize < *alinelen + linelen + 1)
            {
                char *newline = realloc(*aline, *alinelen + linelen + 1);
                if (!newline)
                    err = gpg_error_from_syserror();
                else
                {
                    *aline = newline;
                    gpgsm->colon.attic.linesize += linelen + 1;
                }
            }
            if (!err)
            {
                dst = *aline + *alinelen;
                while (!err && src < end)
                {
                    if (*src == '%' && src + 2 < end)
                    {
                        ++src;
                        *dst = _gpgme_hextobyte(src);
                        (*alinelen)++;
                        src += 2;
                    }
                    else
                    {
                        *dst = *src++;
                        (*alinelen)++;
                    }
                    if (*dst == '\n')
                    {
                        gpgsm->colon.any = 1;
                        if (*alinelen > 1 && *(dst - 1) == '\r')
                            dst--;
                        *dst = '\0';
                        err = gpgsm->colon.fnc(gpgsm->colon.fnc_value, *aline);
                        if (!err)
                        {
                            dst = *aline;
                            *alinelen = 0;
                        }
                    }
                    else
                        dst++;
                }
            }
            TRACE2(DEBUG_CTX, "gpgme:status_handler", gpgsm, "fd 0x%x: D line; final status : % s ", fd, err ? gpg_strerror(err) : " ok ");
        }
        else if (linelen > 2 && line[0] == 'D' && line[1] == ' ' && gpgsm->inline_data)
        {
            char *src = line + 2;
            char *end = line + linelen;
            char *dst = src;
            gpgme_ssize_t nwritten;
            linelen = 0;
            while (src < end)
            {
                if (*src == '%' && src + 2 < end)
                {
                    ++src;
                    *dst++ = _gpgme_hextobyte(src);
                    src += 2;
                }
                else
                    *dst++ = *src++;
                linelen++;
            }
            src = line + 2;
            while (linelen > 0)
            {
                nwritten = gpgme_data_write(gpgsm->inline_data, src, linelen);
                if (!nwritten || (nwritten < 0 && errno != EINTR) || nwritten > linelen)
                {
                    err = gpg_error_from_syserror();
                    break;
                }
                src += nwritten;
                linelen -= nwritten;
            }
            TRACE2(DEBUG_CTX, "gpgme:status_handler", gpgsm, "fd 0x%x: D inlinedata; final status : % s ", fd, err ? gpg_strerror(err) : " ok ");
        }
        else if (linelen > 2 && line[0] == 'S' && line[1] == ' ')
        {
            char *rest;
            gpgme_status_code_t r;
            rest = strchr(line + 2, ' ');
            if (!rest)
                rest = line + linelen;
            else
                *(rest++) = 0;
            r = _gpgme_parse_status(line + 2);
            if (r >= 0)
            {
                if (gpgsm->status.fnc)
                    err = gpgsm->status.fnc(gpgsm->status.fnc_value, r, rest);
            }
            else
                fprintf(stderr, "[UNKNOWN STATUS]%s %s", line + 2, rest);
            TRACE3(DEBUG_CTX, "gpgme:status_handler", gpgsm, "fd 0x%x: S line (%s) - final status: %s", fd, line + 2, err ? gpg_strerror(err) : "ok");
        }
        else if (linelen >= 7 && line[0] == 'I' && line[1] == 'N' && line[2] == 'Q' && line[3] == 'U' && line[4] == 'I' && line[5] == 'R' && line[6] == 'E' && (line[7] == '\0' || line[7] == ' '))
        {
            char *keyword = line + 7;
            while (*keyword == ' ')
                keyword++;
            ;
            default_inq_cb(gpgsm, keyword);
            assuan_write_line(gpgsm->assuan_ctx, "END");
        }
    } while (!err && assuan_pending_line(gpgsm->assuan_ctx));
    return err;
}