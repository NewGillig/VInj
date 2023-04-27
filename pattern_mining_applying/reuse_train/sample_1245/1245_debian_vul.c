static void s390_virtio_register(void)
{
    s390_virtio_bus_register_withprop(&s390_virtio_console);
    s390_virtio_bus_register_withprop(&s390_virtio_blk);
    s390_virtio_bus_register_withprop(&s390_virtio_net);
}