static void *qemu_kvm_cpu_thread_fn(void *arg)
{
    CPUState *cpu = arg;
    int r;
    qemu_mutex_lock(&qemu_global_mutex);
    qemu_thread_get_self(cpu->thread);
    cpu->thread_id = qemu_get_thread_id();
    cpu_single_env = cpu->env_ptr;
    r = kvm_init_vcpu(cpu);
    if (r < 0)
    {
        fprintf(stderr, "kvm_init_vcpu failed: %s\n", strerror(-r));
        exit(1);
    }
    qemu_kvm_init_cpu_signals(cpu);
    cpu->created = true;
    qemu_cond_signal(&qemu_cpu_cond);
    while (1)
    {
        if (cpu_can_run(cpu))
        {
            r = kvm_cpu_exec(cpu);
            if (r == EXCP_DEBUG)
            {
                cpu_handle_guest_debug(cpu);
            }
        }
        qemu_kvm_wait_io_event(cpu);
    }
    return NULL;
}