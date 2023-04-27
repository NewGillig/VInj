static gint dissect_rtp(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
    proto_item *ti = NULL;
    proto_tree *volatile rtp_tree = NULL;
    proto_tree *rtp_csrc_tree = NULL;
    proto_tree *rtp_hext_tree = NULL;
    guint8 octet1, octet2;
    unsigned int version;
    gboolean padding_set;
    gboolean extension_set;
    unsigned int csrc_count;
    gboolean marker_set;
    unsigned int payload_type;
    gchar *payload_type_str = NULL;
    gboolean is_srtp = FALSE;
    unsigned int i = 0;
    unsigned int hdr_extension_len = 0;
    unsigned int hdr_extension_id = 0;
    volatile unsigned int padding_count;
    gint length, reported_length;
    int data_len;
    volatile unsigned int offset = 0;
    guint16 seq_num;
    guint32 timestamp;
    guint32 sync_src;
    guint32 csrc_item;
    struct _rtp_conversation_info *p_conv_data = NULL;
    unsigned int hdrext_offset = 0;
    tvbuff_t *newtvb = NULL;
    const char *pt = NULL;
    static struct _rtp_info rtp_info_arr[4];
    static int rtp_info_current = 0;
    struct _rtp_info *rtp_info;
    rtp_info_current++;
    if (rtp_info_current == 4)
    {
        rtp_info_current = 0;
    }
    rtp_info = &rtp_info_arr[rtp_info_current];
    octet1 = tvb_get_guint8(tvb, offset);
    version = RTP_VERSION(octet1);
    if (version == 0)
    {
        switch (global_rtp_version0_type)
        {
        case RTP0_STUN:
            call_dissector(stun_handle, tvb, pinfo, tree);
            return tvb_length(tvb);
        case RTP0_CLASSICSTUN:
            call_dissector(classicstun_handle, tvb, pinfo, tree);
            return tvb_length(tvb);
        case RTP0_T38:
            call_dissector(t38_handle, tvb, pinfo, tree);
            return tvb_length(tvb);
        case RTP0_SPRT:
            call_dissector(sprt_handle, tvb, pinfo, tree);
            return tvb_length(tvb);
        case RTP0_INVALID:
            if (!(tvb_memeql(tvb, 4, "ZRTP", 4)))
            {
                call_dissector(zrtp_handle, tvb, pinfo, tree);
                return tvb_length(tvb);
            }
        default:;
        }
    }
    rtp_info->info_version = version;
    if (version != 2)
    {
        col_set_str(pinfo->cinfo, COL_PROTOCOL, "RTP");
        col_add_fstr(pinfo->cinfo, COL_INFO, "Unknown RTP version %u", version);
        if (tree)
        {
            ti = proto_tree_add_item(tree, proto_rtp, tvb, offset, -1, ENC_NA);
            rtp_tree = proto_item_add_subtree(ti, ett_rtp);
            proto_tree_add_uint(rtp_tree, hf_rtp_version, tvb, offset, 1, octet1);
        }
        return offset;
    }
    padding_set = RTP_PADDING(octet1);
    extension_set = RTP_EXTENSION(octet1);
    csrc_count = RTP_CSRC_COUNT(octet1);
    octet2 = tvb_get_guint8(tvb, offset + 1);
    marker_set = RTP_MARKER(octet2);
    payload_type = RTP_PAYLOAD_TYPE(octet2);
    if (marker_set && payload_type >= FIRST_RTCP_CONFLICT_PAYLOAD_TYPE && payload_type <= LAST_RTCP_CONFLICT_PAYLOAD_TYPE)
    {
        call_dissector(rtcp_handle, tvb, pinfo, tree);
        return tvb_length(tvb);
    }
    seq_num = tvb_get_ntohs(tvb, offset + 2);
    timestamp = tvb_get_ntohl(tvb, offset + 4);
    sync_src = tvb_get_ntohl(tvb, offset + 8);
    rtp_info->info_padding_set = padding_set;
    rtp_info->info_padding_count = 0;
    rtp_info->info_marker_set = marker_set;
    rtp_info->info_is_video = FALSE;
    rtp_info->info_payload_type = payload_type;
    rtp_info->info_seq_num = seq_num;
    rtp_info->info_timestamp = timestamp;
    rtp_info->info_sync_src = sync_src;
    rtp_info->info_is_srtp = FALSE;
    rtp_info->info_setup_frame_num = 0;
    rtp_info->info_payload_type_str = NULL;
    rtp_info->info_payload_rate = 0;
    length = tvb_length_remaining(tvb, offset);
    reported_length = tvb_reported_length_remaining(tvb, offset);
    if (reported_length >= 0 && length >= reported_length)
    {
        rtp_info->info_all_data_present = TRUE;
        rtp_info->info_data_len = reported_length;
        rtp_info->info_data = tvb_get_ptr(tvb, 0, -1);
    }
    else
    {
        rtp_info->info_all_data_present = FALSE;
        rtp_info->info_data_len = 0;
        rtp_info->info_data = NULL;
    }
    get_conv_info(pinfo, rtp_info);
    p_conv_data = (struct _rtp_conversation_info *)p_get_proto_data(wmem_file_scope(), pinfo, proto_rtp, 0);
    if (p_conv_data)
        rtp_info->info_is_video = p_conv_data->is_video;
    if (p_conv_data && p_conv_data->srtp_info)
        is_srtp = TRUE;
    rtp_info->info_is_srtp = is_srtp;
    col_set_str(pinfo->cinfo, COL_PROTOCOL, (is_srtp) ? "SRTP" : "RTP");
    p_conv_data = (struct _rtp_conversation_info *)p_get_proto_data(wmem_file_scope(), pinfo, proto_rtp, 0);
    srtp_info = p_conv_data->srtp_info;
    if (rtp_info->info_all_data_present)
    {
        srtp_offset = rtp_info->info_data_len - srtp_info->mki_len - srtp_info->auth_tag_len;
    }
}