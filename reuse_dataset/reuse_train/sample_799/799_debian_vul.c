static int dissect_pcp_message(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
    proto_item *root_pcp_item;
    proto_tree *pcp_tree;
    conversation_t *conversation;
    pcp_conv_info_t *pcp_conv_info;
    guint32 packet_type;
    gint32 err_bytes;
    int offset = 0;
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "PCP");
    col_clear(pinfo->cinfo, COL_INFO);
    conversation = find_or_create_conversation(pinfo);
    pcp_conv_info = (pcp_conv_info_t *)conversation_get_proto_data(conversation, proto_pcp);
    if (pcp_conv_info == NULL)
    {
        pcp_conv_info = (pcp_conv_info_t *)g_malloc(sizeof(pcp_conv_info_t));
        conversation_add_proto_data(conversation, proto_pcp, pcp_conv_info);
        pcp_conv_info->pmid_name_candidates = wmem_array_new(wmem_file_scope(), sizeof(guint8 *));
        pcp_conv_info->pmid_to_name = wmem_map_new(wmem_file_scope(), g_direct_hash, g_direct_equal);
        pcp_conv_info->last_pmns_names_frame = 0;
        pcp_conv_info->last_processed_pmns_names_frame = 0;
    }
    root_pcp_item = proto_tree_add_item(tree, proto_pcp, tvb, 0, -1, ENC_NA);
    pcp_tree = proto_item_add_subtree(root_pcp_item, ett_pcp);
    packet_type = tvb_get_ntohl(tvb, 4);
    if (pinfo->srcport == PCP_PORT || pinfo->srcport == PMPROXY_PORT)
    {
        col_set_str(pinfo->cinfo, COL_INFO, "Server > Client ");
    }
    else
    {
        col_set_str(pinfo->cinfo, COL_INFO, "Client > Server ");
    }
    proto_tree_add_item(pcp_tree, hf_pcp_pdu_length, tvb, offset, 4, ENC_BIG_ENDIAN);
    offset += 4;
    proto_tree_add_item(pcp_tree, hf_pcp_pdu_type, tvb, offset, 4, ENC_BIG_ENDIAN);
    offset += 4;
    proto_tree_add_item(pcp_tree, hf_pcp_pdu_pid, tvb, offset, 4, ENC_BIG_ENDIAN);
    offset += 4;
    switch (packet_type)
    {
    case PCP_PDU_CREDS:
        dissect_pcp_message_creds(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_START_OR_ERROR:
        err_bytes = tvb_get_ntohl(tvb, offset);
        if (err_bytes < 0)
        {
            dissect_pcp_message_error(tvb, pinfo, pcp_tree, offset);
        }
        else
        {
            dissect_pcp_message_start(tvb, pinfo, pcp_tree, offset);
        }
        break;
    case PCP_PDU_PMNS_TRAVERSE:
        dissect_pcp_message_pmns_traverse(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_PMNS_NAMES:
        dissect_pcp_message_pmns_names(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_PMNS_CHILD:
        dissect_pcp_message_pmns_child(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_PMNS_IDS:
        dissect_pcp_message_pmns_ids(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_PROFILE:
        dissect_pcp_message_profile(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_FETCH:
        dissect_pcp_message_fetch(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_RESULT:
        dissect_pcp_message_result(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_DESC_REQ:
        dissect_pcp_message_desc_req(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_DESC:
        dissect_pcp_message_desc(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_INSTANCE_REQ:
        dissect_pcp_message_instance_req(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_INSTANCE:
        dissect_pcp_message_instance(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_TEXT_REQ:
        dissect_pcp_message_text_req(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_TEXT:
        dissect_pcp_message_text(tvb, pinfo, pcp_tree, offset);
        break;
    case PCP_PDU_USER_AUTH:
        dissect_pcp_message_user_auth(tvb, pinfo, pcp_tree, offset);
        break;
    default:
        col_append_str(pinfo->cinfo, COL_INFO, "[UNIMPLEMENTED TYPE]");
        expert_add_info(pinfo, pcp_tree, &ei_pcp_unimplemented_packet_type);
        break;
    }
    return tvb_captured_length(tvb);
}