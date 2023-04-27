static int stp_print_mstp_bpdu(netdissect_options *ndo, const struct stp_bpdu_ *stp_bpdu, u_int length)
{
    const u_char *ptr;
    uint16_t v3len;
    uint16_t len;
    uint16_t msti;
    u_int offset;
    ptr = (const u_char *)stp_bpdu;
    ND_PRINT((ndo, ", CIST Flags [%s], length %u", bittok2str(stp_bpdu_flag_values, "none", stp_bpdu->flags), length));
    if (!ndo->ndo_vflag)
    {
        return 1;
    }
    ND_TCHECK(stp_bpdu->flags);
    ND_PRINT((ndo, "\n\tport-role %s, ", tok2str(rstp_obj_port_role_values, "Unknown", RSTP_EXTRACT_PORT_ROLE(stp_bpdu->flags))));
    ND_TCHECK(stp_bpdu->root_path_cost);
    ND_PRINT((ndo, "CIST root-id %s, CIST ext-pathcost %u", stp_print_bridge_id((const u_char *)&stp_bpdu->root_id), EXTRACT_32BITS(&stp_bpdu->root_path_cost)));
    ND_TCHECK(stp_bpdu->bridge_id);
    ND_PRINT((ndo, "\n\tCIST regional-root-id %s, ", stp_print_bridge_id((const u_char *)&stp_bpdu->bridge_id)));
    ND_TCHECK(stp_bpdu->port_id);
    ND_PRINT((ndo, "CIST port-id %04x,", EXTRACT_16BITS(&stp_bpdu->port_id)));
    ND_TCHECK(stp_bpdu->forward_delay);
    ND_PRINT((ndo, "\n\tmessage-age %.2fs, max-age %.2fs"
                   ", hello-time %.2fs, forwarding-delay %.2fs",
              (float)EXTRACT_16BITS(&stp_bpdu->message_age) / STP_TIME_BASE, (float)EXTRACT_16BITS(&stp_bpdu->max_age) / STP_TIME_BASE, (float)EXTRACT_16BITS(&stp_bpdu->hello_time) / STP_TIME_BASE, (float)EXTRACT_16BITS(&stp_bpdu->forward_delay) / STP_TIME_BASE));
    ND_TCHECK_16BITS(ptr + MST_BPDU_VER3_LEN_OFFSET);
    ND_PRINT((ndo, "\n\tv3len %d, ", EXTRACT_16BITS(ptr + MST_BPDU_VER3_LEN_OFFSET)));
    ND_TCHECK_32BITS(ptr + MST_BPDU_CONFIG_DIGEST_OFFSET + 12);
    ND_PRINT((ndo, "MCID Name "));
    if (fn_printzp(ndo, ptr + MST_BPDU_CONFIG_NAME_OFFSET, 32, ndo->ndo_snapend))
        goto trunc;
    ND_PRINT((ndo, ", rev %u,"
                   "\n\t\tdigest %08x%08x%08x%08x, ",
              EXTRACT_16BITS(ptr + MST_BPDU_CONFIG_NAME_OFFSET + 32), EXTRACT_32BITS(ptr + MST_BPDU_CONFIG_DIGEST_OFFSET), EXTRACT_32BITS(ptr + MST_BPDU_CONFIG_DIGEST_OFFSET + 4), EXTRACT_32BITS(ptr + MST_BPDU_CONFIG_DIGEST_OFFSET + 8), EXTRACT_32BITS(ptr + MST_BPDU_CONFIG_DIGEST_OFFSET + 12)));
    ND_TCHECK_32BITS(ptr + MST_BPDU_CIST_INT_PATH_COST_OFFSET);
    ND_PRINT((ndo, "CIST int-root-pathcost %u,", EXTRACT_32BITS(ptr + MST_BPDU_CIST_INT_PATH_COST_OFFSET)));
    ND_TCHECK_BRIDGE_ID(ptr + MST_BPDU_CIST_BRIDGE_ID_OFFSET);
    ND_PRINT((ndo, "\n\tCIST bridge-id %s, ", stp_print_bridge_id(ptr + MST_BPDU_CIST_BRIDGE_ID_OFFSET)));
    ND_TCHECK(ptr[MST_BPDU_CIST_REMAIN_HOPS_OFFSET]);
    ND_PRINT((ndo, "CIST remaining-hops %d", ptr[MST_BPDU_CIST_REMAIN_HOPS_OFFSET]));
    ND_TCHECK_16BITS(ptr + MST_BPDU_VER3_LEN_OFFSET);
    v3len = EXTRACT_16BITS(ptr + MST_BPDU_VER3_LEN_OFFSET);
    if (v3len > MST_BPDU_CONFIG_INFO_LENGTH)
    {
        len = v3len - MST_BPDU_CONFIG_INFO_LENGTH;
        offset = MST_BPDU_MSTI_OFFSET;
        while (len >= MST_BPDU_MSTI_LENGTH)
        {
            ND_TCHECK2(*(ptr + offset), MST_BPDU_MSTI_LENGTH);
            msti = EXTRACT_16BITS(ptr + offset + MST_BPDU_MSTI_ROOT_PRIO_OFFSET);
            msti = msti & 0x0FFF;
            ND_PRINT((ndo, "\n\tMSTI %d, Flags [%s], port-role %s", msti, bittok2str(stp_bpdu_flag_values, "none", ptr[offset]), tok2str(rstp_obj_port_role_values, "Unknown", RSTP_EXTRACT_PORT_ROLE(ptr[offset]))));
            ND_PRINT((ndo, "\n\t\tMSTI regional-root-id %s, pathcost %u", stp_print_bridge_id(ptr + offset + MST_BPDU_MSTI_ROOT_PRIO_OFFSET), EXTRACT_32BITS(ptr + offset + MST_BPDU_MSTI_ROOT_PATH_COST_OFFSET)));
            ND_PRINT((ndo, "\n\t\tMSTI bridge-prio %d, port-prio %d, hops %d", ptr[offset + MST_BPDU_MSTI_BRIDGE_PRIO_OFFSET] >> 4, ptr[offset + MST_BPDU_MSTI_PORT_PRIO_OFFSET] >> 4, ptr[offset + MST_BPDU_MSTI_REMAIN_HOPS_OFFSET]));
            len -= MST_BPDU_MSTI_LENGTH;
            offset += MST_BPDU_MSTI_LENGTH;
        }
    }
    return 1;
trunc:
    return 0;
}