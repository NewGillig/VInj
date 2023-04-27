enum ImapAuthRes imap_auth_cram_md5(struct ImapData *idata, const char *method)
{
    char ibuf[LONG_STRING * 2], obuf[LONG_STRING];
    unsigned char hmac_response[MD5_DIGEST_LEN];
    int len;
    int rc;
    if (!mutt_bit_isset(idata->capabilities, ACRAM_MD5))
        return IMAP_AUTH_UNAVAIL;
    mutt_message(_("Authenticating (CRAM-MD5)..."));
    if (mutt_account_getlogin(&idata->conn->account) < 0)
        return IMAP_AUTH_FAILURE;
    if (mutt_account_getpass(&idata->conn->account) < 0)
        return IMAP_AUTH_FAILURE;
    imap_cmd_start(idata, "AUTHENTICATE CRAM-MD5");
    do
        rc = imap_cmd_step(idata);
    while (rc == IMAP_CMD_CONTINUE);
    if (rc != IMAP_CMD_RESPOND)
    {
        mutt_debug(1, "Invalid response from server: %s\n", ibuf);
        goto bail;
    }
    len = mutt_b64_decode(obuf, idata->buf + 2);
    if (len == -1)
    {
        mutt_debug(1, "Error decoding base64 response.\n");
        goto bail;
    }
    obuf[len] = '\0';
    mutt_debug(2, "CRAM challenge: %s\n", obuf);
    hmac_md5(idata->conn->account.pass, obuf, hmac_response);
    int off = snprintf(obuf, sizeof(obuf), "%s ", idata->conn->account.user);
    mutt_md5_toascii(hmac_response, obuf + off);
    mutt_debug(2, "CRAM response: %s\n", obuf);
    mutt_b64_encode(ibuf, obuf, strlen(obuf), sizeof(ibuf) - 2);
    mutt_str_strcat(ibuf, sizeof(ibuf), "\r\n");
    mutt_socket_send(idata->conn, ibuf);
    do
        rc = imap_cmd_step(idata);
    while (rc == IMAP_CMD_CONTINUE);
    if (rc != IMAP_CMD_OK)
    {
        mutt_debug(1, "Error receiving server response.\n");
        goto bail;
    }
    if (imap_code(idata->buf))
        return IMAP_AUTH_SUCCESS;
bail:
    mutt_error(_("CRAM-MD5 authentication failed."));
    return IMAP_AUTH_FAILURE;
}