static void kq_dealloc(struct event_base *base, void *arg)
{
    struct kqop *kqop = arg;
    if (kqop->changes)
        free(kqop->changes);
    if (kqop->events)
        free(kqop->events);
    if (kqop->kq >= 0 && kqop->pid == getpid())
        close(kqop->kq);
    memset(kqop, 0, sizeof(struct kqop));
    free(kqop);
}