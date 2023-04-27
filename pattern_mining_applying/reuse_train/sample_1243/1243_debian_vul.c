CharDriverState *qdev_init_chardev(DeviceState *dev)
{
    static int next_serial;
    static int next_virtconsole;
    if (strncmp(dev->info->name, "virtio", 6) == 0)
    {
        return virtcon_hds[next_virtconsole++];
    }
    else
    {
        return serial_hds[next_serial++];
    }
}