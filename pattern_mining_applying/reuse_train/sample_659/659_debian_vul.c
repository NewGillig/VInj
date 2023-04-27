int process_netbios_name(const guchar *name_ptr, char *name_ret, int name_ret_len)
{
    int i;
    int name_type = *(name_ptr + NETBIOS_NAME_LEN - 1);
    guchar name_char;
    static const char hex_digits[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    for (i = 0; i < NETBIOS_NAME_LEN - 1; i++)
    {
        name_char = *name_ptr++;
        if (name_char >= ' ' && name_char <= '~')
        {
            if (--name_ret_len > 0)
                *name_ret++ = name_char;
        }
        else
        {
            if (--name_ret_len > 0)
                *name_ret++ = '<';
            if (--name_ret_len > 0)
                *name_ret++ = hex_digits[(name_char >> 4)];
            if (--name_ret_len > 0)
                *name_ret++ = hex_digits[(name_char & 0x0F)];
            if (--name_ret_len > 0)
                *name_ret++ = '>';
        }
    }
    *name_ret = '\0';
    name_ret--;
    for (i = 0; i < NETBIOS_NAME_LEN - 1; i++)
    {
        if (*name_ret != ' ')
        {
            *(name_ret + 1) = 0;
            break;
        }
        name_ret--;
    }
    return name_type;
}