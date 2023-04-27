int event_base_priority_init(struct event_base *base, int npriorities)
{
    int i;
    if (base->event_count_active)
        return (-1);
    if (npriorities == base->nactivequeues)
        return (0);
    if (base->nactivequeues)
    {
        for (i = 0; i < base->nactivequeues; ++i)
        {
            free(base->activequeues[i]);
        }
        free(base->activequeues);
    }
    base->nactivequeues = npriorities;
    base->activequeues = (struct event_list **)calloc(base->nactivequeues, sizeof(struct event_list *));
    if (base->activequeues == NULL)
        event_err(1, "%s: calloc", __func__);
    for (i = 0; i < base->nactivequeues; ++i)
    {
        base->activequeues[i] = malloc(sizeof(struct event_list));
        if (base->activequeues[i] == NULL)
            event_err(1, "%s: malloc", __func__);
        TAILQ_INIT(base->activequeues[i]);
    }
    return (0);
}