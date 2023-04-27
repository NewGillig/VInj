int ImagingFliDecode(Imaging im, ImagingCodecState state, UINT8 *buf, int bytes)
{
    UINT8 *ptr;
    int framesize;
    int c, chunks;
    int l, lines;
    int i, j, x = 0, y, ymax;
    if (bytes < 4)
        return 0;
    ptr = buf;
    framesize = I32(ptr);
    if (framesize < I32(ptr))
        return 0;
    if (I16(ptr + 4) != 0xF1FA)
    {
        state->errcode = IMAGING_CODEC_UNKNOWN;
        return -1;
    }
    chunks = I16(ptr + 6);
    ptr += 16;
    for (c = 0; c < chunks; c++)
    {
        UINT8 *data = ptr + 6;
        switch (I16(ptr + 4))
        {
        case 4:
        case 11:
            break;
        case 7:
            lines = I16(data);
            data += 2;
            for (l = y = 0; l < lines && y < state->ysize; l++, y++)
            {
                UINT8 *buf = (UINT8 *)im->image[y];
                int p, packets;
                packets = I16(data);
                data += 2;
                while (packets & 0x8000)
                {
                    if (packets & 0x4000)
                    {
                        y += 65536 - packets;
                        if (y >= state->ysize)
                        {
                            state->errcode = IMAGING_CODEC_OVERRUN;
                            return -1;
                        }
                        buf = (UINT8 *)im->image[y];
                    }
                    else
                    {
                        buf[state->xsize - 1] = (UINT8)packets;
                    }
                    packets = I16(data);
                    data += 2;
                }
                for (p = x = 0; p < packets; p++)
                {
                    x += data[0];
                    if (data[1] >= 128)
                    {
                        i = 256 - data[1];
                        if (x + i + i > state->xsize)
                            break;
                        for (j = 0; j < i; j++)
                        {
                            buf[x++] = data[2];
                            buf[x++] = data[3];
                        }
                        data += 2 + 2;
                    }
                    else
                    {
                        i = 2 * (int)data[1];
                        if (x + i > state->xsize)
                            break;
                        memcpy(buf + x, data + 2, i);
                        data += 2 + i;
                        x += i;
                    }
                }
                if (p < packets)
                    break;
            }
            if (l < lines)
            {
                state->errcode = IMAGING_CODEC_OVERRUN;
                return -1;
            }
            break;
        case 12:
            y = I16(data);
            ymax = y + I16(data + 2);
            data += 4;
            for (; y < ymax && y < state->ysize; y++)
            {
                UINT8 *out = (UINT8 *)im->image[y];
                int p, packets = *data++;
                for (p = x = 0; p < packets; p++, x += i)
                {
                    x += data[0];
                    if (data[1] & 0x80)
                    {
                        i = 256 - data[1];
                        if (x + i > state->xsize)
                            break;
                        memset(out + x, data[2], i);
                        data += 3;
                    }
                    else
                    {
                        i = data[1];
                        if (x + i > state->xsize)
                            break;
                        memcpy(out + x, data + 2, i);
                        data += i + 2;
                    }
                }
                if (p < packets)
                    break;
            }
            if (y < ymax)
            {
                state->errcode = IMAGING_CODEC_OVERRUN;
                return -1;
            }
            break;
        case 13:
            for (y = 0; y < state->ysize; y++)
                memset(im->image[y], 0, state->xsize);
            break;
        case 15:
            for (y = 0; y < state->ysize; y++)
            {
                UINT8 *out = (UINT8 *)im->image[y];
                data += 1;
                for (x = 0; x < state->xsize; x += i)
                {
                    if (data[0] & 0x80)
                    {
                        i = 256 - data[0];
                        if (x + i > state->xsize)
                            break;
                        memcpy(out + x, data + 1, i);
                        data += i + 1;
                    }
                    else
                    {
                        i = data[0];
                        if (x + i > state->xsize)
                            break;
                        memset(out + x, data[1], i);
                        data += 2;
                    }
                }
                if (x != state->xsize)
                {
                    state->errcode = IMAGING_CODEC_OVERRUN;
                    return -1;
                }
            }
            break;
        case 16:
            for (y = 0; y < state->ysize; y++)
            {
                UINT8 *buf = (UINT8 *)im->image[y];
                memcpy(buf, data, state->xsize);
                data += state->xsize;
            }
            break;
        case 18:
            break;
        default:
            state->errcode = IMAGING_CODEC_UNKNOWN;
            return -1;
        }
        ptr += I32(ptr);
    }
    return -1;
}