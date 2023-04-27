static void cirrus_mem_writeb_mode4and5_16bpp(CirrusVGAState *s, unsigned mode, unsigned offset, uint32_t mem_value)
{
    int x;
    unsigned val = mem_value;
    uint8_t *dst;
    for (x = 0; x < 8; x++)
    {
        dst = s->vga.vram_ptr + ((offset + 2 * x) & s->cirrus_addr_mask & ~1);
        if (val & 0x80)
        {
            *dst = s->cirrus_shadow_gr1;
            *(dst + 1) = s->vga.gr[0x11];
        }
        else if (mode == 5)
        {
            *dst = s->cirrus_shadow_gr0;
            *(dst + 1) = s->vga.gr[0x10];
        }
        val <<= 1;
    }
    memory_region_set_dirty(&s->vga.vram, offset, 16);
}