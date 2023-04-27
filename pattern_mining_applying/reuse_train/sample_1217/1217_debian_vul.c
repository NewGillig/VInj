static int s_aes_process(stream_state *ss, stream_cursor_read *pr, stream_cursor_write *pw, bool last)
{
    stream_aes_state *const state = (stream_aes_state *)ss;
    const unsigned char *limit;
    const long in_size = pr->limit - pr->ptr;
    const long out_size = pw->limit - pw->ptr;
    unsigned char temp[16];
    int status = 0;
    if (in_size > out_size)
    {
        limit = pr->ptr + out_size;
        status = 1;
    }
    else
    {
        limit = pr->limit;
        status = last ? EOFC : 0;
    }
    if (state->ctx == NULL)
    {
        state->ctx = (aes_context *)gs_alloc_bytes_immovable(state->memory, sizeof(aes_context), "aes context structure");
        if (state->ctx == NULL)
        {
            gs_throw(gs_error_VMerror, "could not allocate aes context");
            return ERRC;
        }
        if (state->keylength < 1 || state->keylength > SAES_MAX_KEYLENGTH)
        {
            gs_throw1(gs_error_rangecheck, "invalid aes key length (%d bytes)", state->keylength);
            return ERRC;
        }
        aes_setkey_dec(state->ctx, state->key, state->keylength * 8);
    }
    if (!state->initialized)
    {
        if (in_size < 16)
            return 0;
        memcpy(state->iv, pr->ptr + 1, 16);
        state->initialized = 1;
        pr->ptr += 16;
    }
    while (pr->ptr + 16 <= limit)
    {
        aes_crypt_cbc(state->ctx, AES_DECRYPT, 16, state->iv, pr->ptr + 1, temp);
        pr->ptr += 16;
        if (last && pr->ptr == pr->limit)
        {
            int pad;
            if (state->use_padding)
            {
                pad = temp[15];
                if (pad < 1 || pad > 16)
                {
                    gs_warn1("invalid aes padding byte (0x%02x)", (unsigned char)pad);
                    pad = 0;
                }
            }
            else
            {
                pad = 0;
            }
            memcpy(pw->ptr + 1, temp, 16 - pad);
            pw->ptr += 16 - pad;
            return EOFC;
        }
        memcpy(pw->ptr + 1, temp, 16);
        pw->ptr += 16;
    }
    if (status == EOFC)
    {
        gs_throw(gs_error_rangecheck, "aes stream isn't a multiple of 16 bytes");
        return 0;
    }
    return status;
}