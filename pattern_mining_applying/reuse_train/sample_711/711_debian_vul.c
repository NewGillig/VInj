static void save_command(guint32 cmd, guint32 arg0, guint32 arg1, guint32 data_length, guint32 crc32, service_data_t *service_data, gint proto, void *data, packet_info *pinfo, service_data_t **returned_service_data, command_data_t **returned_command_data)
{
    wmem_tree_key_t key[6];
    guint32 interface_id;
    guint32 bus_id;
    guint32 device_address;
    guint32 side_id;
    guint32 frame_number;
    command_data_t *command_data;
    wmem_tree_t *wmem_tree;
    gint direction = P2P_DIR_UNKNOWN;
    usb_conv_info_t *usb_conv_info = (usb_conv_info_t *)data;
    frame_number = pinfo->num;
    if (pinfo->phdr->presence_flags & WTAP_HAS_INTERFACE_ID)
        interface_id = pinfo->phdr->interface_id;
    else
        interface_id = 0;
    if (proto == proto_usb)
    {
        usb_conv_info = (usb_conv_info_t *)data;
        DISSECTOR_ASSERT(usb_conv_info);
        direction = usb_conv_info->direction;
        bus_id = usb_conv_info->bus_id;
        device_address = usb_conv_info->device_address;
        key[0].length = 1;
        key[0].key = &interface_id;
        key[1].length = 1;
        key[1].key = &bus_id;
        key[2].length = 1;
        key[2].key = &device_address;
        key[3].length = 1;
        key[3].key = &side_id;
        key[4].length = 1;
        key[4].key = &frame_number;
        key[5].length = 0;
        key[5].key = NULL;
    }
    else
    {
        if (pinfo->destport == ADB_TCP_PORT)
            direction = P2P_DIR_SENT;
        else
            direction = P2P_DIR_RECV;
        key[0].length = 1;
        key[0].key = &interface_id;
        key[1].length = 1;
        key[2].length = 1;
        if (direction == P2P_DIR_SENT)
        {
            key[1].key = &pinfo->srcport;
            key[2].key = &pinfo->destport;
        }
        else
        {
            key[1].key = &pinfo->destport;
            key[2].key = &pinfo->srcport;
        }
        key[3].length = 1;
        key[3].key = &side_id;
        key[4].length = 1;
        key[4].key = &frame_number;
        key[5].length = 0;
        key[5].key = NULL;
    }
    if (direction == P2P_DIR_SENT)
        if (cmd == A_CLSE)
            side_id = arg1;
        else
            side_id = arg0;
    else
        side_id = arg1;
    if (cmd == A_OPEN)
    {
        service_data = wmem_new(wmem_file_scope(), service_data_t);
        service_data->start_in_frame = pinfo->num;
        service_data->close_local_in_frame = max_in_frame;
        service_data->close_remote_in_frame = max_in_frame;
        service_data->local_id = arg0;
        service_data->remote_id = arg1;
        service_data->service = "unknown";
        wmem_tree_insert32_array(service_info, key, service_data);
    }
    command_data = wmem_new(wmem_file_scope(), command_data_t);
    command_data->command = cmd;
    command_data->arg0 = arg0;
    command_data->arg1 = arg1;
    command_data->command_in_frame = pinfo->num;
    command_data->response_in_frame = max_in_frame;
    command_data->crc32 = crc32;
    command_data->data_length = data_length;
    if (data_length == 0)
        command_data->completed_in_frame = pinfo->num;
    else
        command_data->completed_in_frame = max_in_frame;
    command_data->reassemble_data_length = 0;
    command_data->reassemble_data = (guint8 *)wmem_alloc(wmem_file_scope(), command_data->data_length);
    key[3].length = 1;
    key[3].key = &frame_number;
    key[4].length = 0;
    key[4].key = NULL;
    wmem_tree_insert32_array(command_info, key, command_data);
    if (direction == P2P_DIR_SENT)
        if (command_data->command == A_CLSE)
            side_id = command_data->arg1;
        else
            side_id = command_data->arg0;
    else
        side_id = command_data->arg1;
    key[3].length = 1;
    key[3].key = &side_id;
    key[4].length = 0;
    key[4].key = NULL;
    wmem_tree = (wmem_tree_t *)wmem_tree_lookup32_array(service_info, key);
    if (wmem_tree)
    {
        service_data = (service_data_t *)wmem_tree_lookup32_le(wmem_tree, frame_number);
    }
    if (cmd == A_OKAY)
    {
        if (!service_data)
        {
            if (direction == P2P_DIR_SENT)
                side_id = command_data->arg0;
            else
                side_id = command_data->arg1;
            wmem_tree = (wmem_tree_t *)wmem_tree_lookup32_array(service_info, key);
            if (wmem_tree)
            {
                service_data = (service_data_t *)wmem_tree_lookup32_le(wmem_tree, frame_number);
            }
        }
        if (service_data && service_data->remote_id == 0 && direction == P2P_DIR_RECV)
        {
            if (direction == P2P_DIR_SENT)
            {
                service_data->remote_id = arg1;
            }
            else
            {
                service_data->remote_id = arg0;
            }
            side_id = service_data->remote_id;
            key[4].length = 1;
            key[4].key = &frame_number;
            key[5].length = 0;
            key[5].key = NULL;
            wmem_tree_insert32_array(service_info, key, service_data);
        }
    }
    else if (cmd == A_CLSE)
    {
        if (service_data)
        {
            if (direction == P2P_DIR_RECV && service_data->local_id == arg1)
                service_data->close_local_in_frame = pinfo->num;
            else if (direction == P2P_DIR_SENT && service_data->remote_id == arg1)
                service_data->close_remote_in_frame = pinfo->num;
        }
    }
    DISSECTOR_ASSERT(returned_service_data && returned_command_data);
    *returned_service_data = service_data;
    *returned_command_data = command_data;
}