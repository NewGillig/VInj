static inline int unreference_pic(H264Context *h, Picture *pic, int refmask)
{
    int i;
    if (pic->f.reference &= refmask)
    {
        return 0;
    }
    else
    {
        for (i = 0; h->delayed_pic[i]; i++)
            if (pic == h->delayed_pic[i])
            {
                pic->f.reference = DELAYED_PIC_REF;
                break;
            }
        return 1;
    }
}