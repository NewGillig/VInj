static int dissect_CPMSetBindings(tvbuff_t *tvb, packet_info *pinfo, proto_tree *parent_tree, gboolean in, void *private_data)
{
    gint offset = 16;
    struct CPMSetBindingsIn request;
    col_append_str(pinfo->cinfo, COL_INFO, "SetBindings");
    if (in)
    {
        struct mswsp_ct *ct = NULL;
        struct message_data *data = NULL;
        proto_item *ti;
        proto_tree *tree, *pad_tree;
        guint32 size, num, n;
        gint64 column_size;
        ti = proto_tree_add_item(parent_tree, hf_mswsp_msg, tvb, offset, -1, ENC_NA);
        tree = proto_item_add_subtree(ti, ett_mswsp_msg);
        proto_item_set_text(ti, "SetBindingsIn");
        pad_tree = proto_tree_add_subtree(tree, tvb, offset, 0, ett_mswsp_pad, &ti, "Padding");
        proto_tree_add_item(tree, hf_mswsp_msg_cpmsetbinding_hcursor, tvb, offset, 4, ENC_LITTLE_ENDIAN);
        request.hcursor = tvb_get_letohl(tvb, offset);
        offset += 4;
        request.brow = tvb_get_letohl(tvb, offset);
        proto_tree_add_item(tree, hf_mswsp_msg_cpmsetbinding_cbrow, tvb, offset, 4, ENC_LITTLE_ENDIAN);
        offset += 4;
        size = tvb_get_letohl(tvb, offset);
        request.bbindingdesc = size;
        proto_tree_add_item(tree, hf_mswsp_msg_cpmsetbinding_desc, tvb, offset, 4, ENC_LITTLE_ENDIAN);
        offset += 4;
        request.dummy = tvb_get_letohl(tvb, offset);
        proto_tree_add_item(tree, hf_mswsp_msg_cpmsetbinding_dummy, tvb, offset, 4, ENC_LITTLE_ENDIAN);
        offset += 4;
        num = tvb_get_letohl(tvb, offset);
        request.ccolumns = num;
        ti = proto_tree_add_item(tree, hf_mswsp_msg_cpmsetbinding_ccolumns, tvb, offset, 4, ENC_LITTLE_ENDIAN);
        offset += 4;
        proto_tree_add_item(tree, hf_mswsp_msg_cpmsetbinding_acolumns, tvb, offset, size - 4, ENC_NA);
        column_size = num * MIN_CTABLECOL_SIZE;
        if (column_size > tvb_reported_length_remaining(tvb, offset))
        {
            expert_add_info(pinfo, ti, &ei_mswsp_msg_cpmsetbinding_ccolumns);
            return tvb_reported_length(tvb);
        }
        ct = get_create_converstation_data(pinfo);
        request.acolumns = (struct CTableColumn *)wmem_alloc(wmem_file_scope(), sizeof(struct CTableColumn) * num);
        for (n = 0; n < num; n++)
        {
            offset = parse_padding(tvb, offset, 4, pad_tree, "padding_aColumns[%u]", n);
            offset = parse_CTableColumn(tvb, offset, tree, pad_tree, &request.acolumns[n], "aColumns[%u]", n);
        }
        data = find_or_create_message_data(ct, pinfo, 0xD0, in, private_data);
        if (data)
        {
            data->content.bindingsin = request;
        }
    }
    else
    {
    }
    return tvb_reported_length(tvb);
}