void min_heap_dtor(min_heap_t *s)
{
    if (s->p)
        free(s->p);
}