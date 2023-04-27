static int get_current_cpu(void)
{
    CPUState *cpu_single_cpu;
    if (!cpu_single_env)
    {
        return -1;
    }
    cpu_single_cpu = ENV_GET_CPU(cpu_single_env);
    return cpu_single_cpu->cpu_index;
}