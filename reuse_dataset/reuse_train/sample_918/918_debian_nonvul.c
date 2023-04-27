static ulong get_sort(uint count, ...)
{
    va_list args;
    va_start(args, count);
    ulong sort = 0;
    DBUG_ASSERT(count <= 4);
    while (count--)
    {
        char *start, *str = va_arg(args, char *);
        uint chars = 0;
        uint wild_pos = 0;
        if ((start = str))
        {
            for (; *str; str++)
            {
                if (*str == wild_prefix && str[1])
                    str++;
                else if (*str == wild_many || *str == wild_one)
                {
                    wild_pos = (uint)(str - start) + 1;
                    if (!(wild_pos == 1 && *str == wild_many && *(++str) == '\0'))
                        wild_pos++;
                    break;
                }
                chars = 128;
            }
        }
        sort = (sort << 8) + (wild_pos ? min(wild_pos, 127) : chars);
    }
    va_end(args);
    return sort;
}