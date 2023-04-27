static inline int gic_get_current_cpu(GICState *s)
{
    if (s->num_cpu > 1)
    {
        CPUState *cpu = ENV_GET_CPU(cpu_single_env);
        return cpu->cpu_index;
    }
    return 0;
}