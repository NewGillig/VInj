static int decode_udvm_multitype_operand(guint8 *buff, guint operand_address, guint16 *value)
{
    guint test_bits;
    guint bytecode;
    guint offset = operand_address;
    guint16 operand;
    guint32 result;
    guint8 temp_data;
    guint16 temp_data16;
    guint16 memmory_addr = 0;
    *value = 0;
    if (operand_address >= UDVM_MEMORY_SIZE)
        return -1;
    bytecode = buff[operand_address];
    test_bits = (bytecode & 0xc0) >> 6;
    switch (test_bits)
    {
    case 0:
        operand = buff[operand_address];
        *value = operand;
        offset++;
        break;
    case 1:
        memmory_addr = (bytecode & 0x3f) * 2;
        temp_data16 = buff[memmory_addr] << 8;
        temp_data16 = temp_data16 | buff[(memmory_addr + 1) & 0xffff];
        *value = temp_data16;
        offset++;
        break;
    case 2:
        test_bits = (bytecode & 0xe0) >> 5;
        if (test_bits == 5)
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
            test_bits = (bytecode & 0xf0) >> 4;
            if (test_bits == 9)
            {
                temp_data = buff[operand_address] & 0x0f;
                operand = temp_data << 8;
                temp_data = buff[(operand_address + 1) & 0xffff];
                operand = operand | temp_data;
                operand = operand + 61440;
                *value = operand;
                offset = offset + 2;
            }
            else
            {
                test_bits = (bytecode & 0x08) >> 3;
                if (test_bits == 1)
                {
                    result = 1 << ((buff[operand_address] & 0x07) + 8);
                    operand = result & 0xffff;
                    *value = operand;
                    offset++;
                }
                else
                {
                    test_bits = (bytecode & 0x0e) >> 1;
                    if (test_bits == 3)
                    {
                        result = 1 << ((buff[operand_address] & 0x01) + 6);
                        operand = result & 0xffff;
                        *value = operand;
                        offset++;
                    }
                    else
                    {
                        offset++;
                        temp_data16 = buff[(operand_address + 1) & 0xffff] << 8;
                        temp_data16 = temp_data16 | buff[(operand_address + 2) & 0xffff];
                        if ((bytecode & 0x01) == 1)
                        {
                            memmory_addr = temp_data16;
                            temp_data16 = buff[memmory_addr] << 8;
                            temp_data16 = temp_data16 | buff[(memmory_addr + 1) & 0xffff];
                        }
                        *value = temp_data16;
                        offset = offset + 2;
                    }
                }
            }
        }
        break;
    case 3:
        test_bits = (bytecode & 0x20) >> 5;
        if (test_bits == 1)
        {
            operand = (buff[operand_address] & 0x1f) + 65504;
            *value = operand;
            offset++;
        }
        else
        {
            memmory_addr = buff[operand_address] & 0x1f;
            memmory_addr = memmory_addr << 8;
            memmory_addr = memmory_addr | buff[(operand_address + 1) & 0xffff];
            temp_data16 = buff[memmory_addr] << 8;
            temp_data16 = temp_data16 | buff[(memmory_addr + 1) & 0xffff];
            *value = temp_data16;
            offset = offset + 2;
        }
    default:
        break;
    }
    return offset;
}