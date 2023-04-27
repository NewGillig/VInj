void bluetooth_add_address(packet_info *pinfo, address *addr, const gchar *setup_method, guint32 setup_frame_number, gboolean is_video, void *data)
{
    address null_addr;
    conversation_t *p_conv;
    struct _rtp_conversation_info *p_conv_data = NULL;
    if ((pinfo->fd->flags.visited) || (rtp_handle == NULL))
    {
        return;
    }
    SET_ADDRESS(&null_addr, AT_NONE, 0, NULL);
    p_conv = find_conversation(setup_frame_number, addr, &null_addr, PT_BLUETOOTH, 0, 0, NO_ADDR_B | NO_PORT_B);
    if (!p_conv || p_conv->setup_frame != setup_frame_number)
    {
        p_conv = conversation_new(setup_frame_number, addr, &null_addr, PT_BLUETOOTH, 0, 0, NO_ADDR2 | NO_PORT2);
    }
    conversation_set_dissector(p_conv, rtp_handle);
    p_conv_data = (struct _rtp_conversation_info *)conversation_get_proto_data(p_conv, proto_rtp);
    if (!p_conv_data)
    {
        p_conv_data = wmem_new(wmem_file_scope(), struct _rtp_conversation_info);
        p_conv_data->rtp_dyn_payload = NULL;
        p_conv_data->extended_seqno = 0x10000;
        p_conv_data->rtp_conv_info = wmem_new(wmem_file_scope(), rtp_private_conv_info);
        p_conv_data->rtp_conv_info->multisegment_pdus = wmem_tree_new(wmem_file_scope());
        conversation_add_proto_data(p_conv, proto_rtp, p_conv_data);
        if (is_video)
        {
            p_conv_data->bta2dp_info = NULL;
            p_conv_data->btvdp_info = (btvdp_codec_info_t *)wmem_memdup(wmem_file_scope(), data, sizeof(btvdp_codec_info_t));
        }
        else
        {
            p_conv_data->bta2dp_info = (bta2dp_codec_info_t *)wmem_memdup(wmem_file_scope(), data, sizeof(bta2dp_codec_info_t));
            p_conv_data->btvdp_info = NULL;
        }
    }
    rtp_dyn_payload_free(p_conv_data->rtp_dyn_payload);
    g_strlcpy(p_conv_data->method, setup_method, MAX_RTP_SETUP_METHOD_SIZE + 1);
    p_conv_data->frame_number = setup_frame_number;
    p_conv_data->is_video = is_video;
    p_conv_data->rtp_dyn_payload = NULL;
    p_conv_data->srtp_info = NULL;
}