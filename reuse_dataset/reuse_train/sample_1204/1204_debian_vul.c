static void cpu_signal(int sig)
{
    if (cpu_single_env)
    {
        cpu_exit(ENV_GET_CPU(cpu_single_env));
    }
    exit_request = 1;
}