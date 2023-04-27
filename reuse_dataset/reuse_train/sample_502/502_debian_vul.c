static int vp3_init_thread_copy(AVCodecContext *avctx)
{
    Vp3DecodeContext *s = avctx->priv_data;
    s->superblock_coding = NULL;
    s->all_fragments = NULL;
    s->coded_fragment_list[0] = NULL;
    s->dct_tokens_base = NULL;
    s->superblock_fragments = NULL;
    s->macroblock_coding = NULL;
    s->motion_val[0] = NULL;
    s->motion_val[1] = NULL;
    s->edge_emu_buffer = NULL;
    return 0;
}