static void cpu_signal(int sig)
{
    if (current_cpu)
    {
        cpu_exit(current_cpu);
    }
    exit_request = 1;
}