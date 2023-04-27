static int decode_udvm_literal_operand(guint8 *buff, guint operand_address, guint16 *value)
{
    guint bytecode;
    guint16 operand;
    guint test_bits;
    guint offset = operand_address;
    guint8 temp_data;
    if (operand_address >= UDVM_MEMORY_SIZE)
        return -1;
    bytecode = buff[operand_address];
    test_bits = bytecode >> 7;
    if (test_bits == 1)
    {
        test_bits = bytecode >> 6;
        if (test_bits == 2)
        {
            temp_data = buff[operand_address] & 0x1f;
            operand = temp_data << 8;
            temp_data = buff[(operand_address + 1) & 0xffff];
            operand = operand | temp_data;
            *value = operand;
            offset = offset + 2;
        }
        else
        {
            offset++;
            temp_data = buff[operand_address] & 0x1f;
            operand = temp_data << 8;
            temp_data = buff[(operand_address + 1) & 0xffff];
            operand = operand | temp_data;
            *value = operand;
            offset = offset + 2;
        }
    }
    else
    {
        operand = (bytecode & 0x7f);
        *value = operand;
        offset++;
    }
    return offset;
}