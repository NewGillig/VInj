static void *kq_init(struct event_base *base)
{
    int i, kq;
    struct kqop *kqueueop;
    if (evutil_getenv("EVENT_NOKQUEUE"))
        return (NULL);
    if (!(kqueueop = calloc(1, sizeof(struct kqop))))
        return (NULL);
    if ((kq = kqueue()) == -1)
    {
        event_warn("kqueue");
        free(kqueueop);
        return (NULL);
    }
    kqueueop->kq = kq;
    kqueueop->pid = getpid();
    kqueueop->changes = malloc(NEVENT * sizeof(struct kevent));
    if (kqueueop->changes == NULL)
    {
        free(kqueueop);
        return (NULL);
    }
    kqueueop->events = malloc(NEVENT * sizeof(struct kevent));
    if (kqueueop->events == NULL)
    {
        free(kqueueop->changes);
        free(kqueueop);
        return (NULL);
    }
    kqueueop->nevents = NEVENT;
    for (i = 0; i < NSIG; ++i)
    {
        TAILQ_INIT(&kqueueop->evsigevents[i]);
    }
    kqueueop->changes[0].ident = -1;
    kqueueop->changes[0].filter = EVFILT_READ;
    kqueueop->changes[0].flags = EV_ADD;
    if (kevent(kq, kqueueop->changes, 1, kqueueop->events, NEVENT, NULL) != 1 || kqueueop->events[0].ident != -1 || kqueueop->events[0].flags != EV_ERROR)
    {
        event_warn("%s: detected broken kqueue; not using . ", __func__);
        free(kqueueop->changes);
        free(kqueueop->events);
        free(kqueueop);
        close(kq);
        return (NULL);
    }
    return (kqueueop);
}