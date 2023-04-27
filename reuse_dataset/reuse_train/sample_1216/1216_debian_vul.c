void stp_print(netdissect_options *ndo, const u_char *p, u_int length)
{
    const struct stp_bpdu_ *stp_bpdu;
    u_int mstp_len;
    u_int spb_len;
    stp_bpdu = (const struct stp_bpdu_ *)p;
    if (length < 4)
        goto trunc;
    ND_TCHECK(stp_bpdu->protocol_id);
    if (EXTRACT_16BITS(&stp_bpdu->protocol_id))
    {
        ND_PRINT((ndo, "unknown STP version, length %u", length));
        return;
    }
    ND_TCHECK(stp_bpdu->protocol_version);
    ND_PRINT((ndo, "STP %s", tok2str(stp_proto_values, "Unknown STP protocol (0x%02x)", stp_bpdu->protocol_version)));
    switch (stp_bpdu->protocol_version)
    {
    case STP_PROTO_REGULAR:
    case STP_PROTO_RAPID:
    case STP_PROTO_MSTP:
    case STP_PROTO_SPB:
        break;
    default:
        return;
    }
    ND_TCHECK(stp_bpdu->bpdu_type);
    ND_PRINT((ndo, ", %s", tok2str(stp_bpdu_type_values, "Unknown BPDU Type (0x%02x)", stp_bpdu->bpdu_type)));
    switch (stp_bpdu->bpdu_type)
    {
    case STP_BPDU_TYPE_CONFIG:
        if (length < sizeof(struct stp_bpdu_) - 1)
        {
            goto trunc;
        }
        if (!stp_print_config_bpdu(ndo, stp_bpdu, length))
            goto trunc;
        break;
    case STP_BPDU_TYPE_RSTP:
        if (stp_bpdu->protocol_version == STP_PROTO_RAPID)
        {
            if (length < sizeof(struct stp_bpdu_))
            {
                goto trunc;
            }
            if (!stp_print_config_bpdu(ndo, stp_bpdu, length))
                goto trunc;
        }
        else if (stp_bpdu->protocol_version == STP_PROTO_MSTP || stp_bpdu->protocol_version == STP_PROTO_SPB)
        {
            if (length < STP_BPDU_MSTP_MIN_LEN)
            {
                goto trunc;
            }
            ND_TCHECK(stp_bpdu->v1_length);
            if (stp_bpdu->v1_length != 0)
            {
                goto trunc;
            }
            ND_TCHECK_16BITS(p + MST_BPDU_VER3_LEN_OFFSET);
            mstp_len = EXTRACT_16BITS(p + MST_BPDU_VER3_LEN_OFFSET);
            mstp_len += 2;
            if (length < (sizeof(struct stp_bpdu_) + mstp_len))
            {
                goto trunc;
            }
            if (!stp_print_mstp_bpdu(ndo, stp_bpdu, length))
                goto trunc;
            if (stp_bpdu->protocol_version == STP_PROTO_SPB)
            {
                spb_len = EXTRACT_16BITS(p + MST_BPDU_VER3_LEN_OFFSET + mstp_len);
                spb_len += 2;
                if (length < (sizeof(struct stp_bpdu_) + mstp_len + spb_len) || spb_len < SPB_BPDU_MIN_LEN)
                {
                    goto trunc;
                }
                if (!stp_print_spb_bpdu(ndo, stp_bpdu, (sizeof(struct stp_bpdu_) + mstp_len)))
                    goto trunc;
            }
        }
        break;
    case STP_BPDU_TYPE_TOPO_CHANGE:
        break;
    default:
        break;
    }
    return;
trunc:
    ND_PRINT((ndo, "[|stp %d]", length));
}