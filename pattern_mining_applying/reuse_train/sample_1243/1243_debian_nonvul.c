CharDriverState *qdev_init_chardev(DeviceState *dev)
{
    static int next_serial;
    return serial_hds[next_serial++];
}