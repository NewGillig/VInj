static bool cgroupfs_mount_cgroup(void *hdata, const char *root, int type)
{
    size_t bufsz = strlen(root) + sizeof("/sys/fs/cgroup");
    char *path = NULL;
    char **parts = NULL;
    char *dirname = NULL;
    char *abs_path = NULL;
    char *abs_path2 = NULL;
    struct cgfs_data *cgfs_d;
    struct cgroup_process_info *info, *base_info;
    int r, saved_errno = 0;
    cgfs_d = hdata;
    if (!cgfs_d)
        return false;
    base_info = cgfs_d->info;
    if (type == LXC_AUTO_CGROUP_FULL_NOSPEC)
        type = LXC_AUTO_CGROUP_FULL_MIXED;
    else if (type == LXC_AUTO_CGROUP_NOSPEC)
        type = LXC_AUTO_CGROUP_MIXED;
    if (type < LXC_AUTO_CGROUP_RO || type > LXC_AUTO_CGROUP_FULL_MIXED)
    {
        ERROR("could not mount cgroups into container: invalid type specified internally");
        errno = EINVAL;
        return false;
    }
    path = calloc(1, bufsz);
    if (!path)
        return false;
    snprintf(path, bufsz, "%s/sys/fs/cgroup", root);
    r = safe_mount("cgroup_root", path, "tmpfs", MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_RELATIME, "size=10240k,mode=755", root);
    if (r < 0)
    {
        SYSERROR("could not mount tmpfs to /sys/fs/cgroup in the container");
        return false;
    }
    for (info = base_info; info; info = info->next)
    {
        size_t subsystem_count, i;
        struct cgroup_mount_point *mp = info->designated_mount_point;
        if (!mp)
            mp = lxc_cgroup_find_mount_point(info->hierarchy, info->cgroup_path, true);
        if (!mp)
        {
            SYSERROR("could not find original mount point for cgroup hierarchy while trying to mount cgroup filesystem");
            goto out_error;
        }
        subsystem_count = lxc_array_len((void **)info->hierarchy->subsystems);
        parts = calloc(subsystem_count + 1, sizeof(char *));
        if (!parts)
            goto out_error;
        for (i = 0; i < subsystem_count; i++)
        {
            if (!strncmp(info->hierarchy->subsystems[i], "name=", 5))
                parts[i] = info->hierarchy->subsystems[i] + 5;
            else
                parts[i] = info->hierarchy->subsystems[i];
        }
        dirname = lxc_string_join(",", (const char **)parts, false);
        if (!dirname)
            goto out_error;
        abs_path = lxc_append_paths(path, dirname);
        if (!abs_path)
            goto out_error;
        r = mkdir_p(abs_path, 0755);
        if (r < 0 && errno != EEXIST)
        {
            SYSERROR("could not create cgroup subsystem directory /sys/fs/cgroup/%s", dirname);
            goto out_error;
        }
        abs_path2 = lxc_append_paths(abs_path, info->cgroup_path);
        if (!abs_path2)
            goto out_error;
        if (type == LXC_AUTO_CGROUP_FULL_RO || type == LXC_AUTO_CGROUP_FULL_RW || type == LXC_AUTO_CGROUP_FULL_MIXED)
        {
            if (strcmp(mp->mount_prefix, "/") != 0)
            {
                ERROR("could not automatically mount cgroup-full to /sys/fs/cgroup/%s: host has no mount point for this cgroup filesystem that has access to the root cgroup", dirname);
                goto out_error;
            }
            r = mount(mp->mount_point, abs_path, "none", MS_BIND, 0);
            if (r < 0)
            {
                SYSERROR("error bind-mounting %s to %s", mp->mount_point, abs_path);
                goto out_error;
            }
            if (type == LXC_AUTO_CGROUP_FULL_RO || type == LXC_AUTO_CGROUP_FULL_MIXED)
            {
                r = mount(NULL, abs_path, NULL, MS_REMOUNT | MS_BIND | MS_RDONLY, NULL);
                if (r < 0)
                {
                    SYSERROR("error re-mounting %s readonly", abs_path);
                    goto out_error;
                }
            }
            if (type == LXC_AUTO_CGROUP_FULL_MIXED)
            {
                r = mount(abs_path2, abs_path2, NULL, MS_BIND, NULL);
                if (r < 0)
                {
                    SYSERROR("error bind-mounting %s onto itself", abs_path2);
                    goto out_error;
                }
                r = mount(NULL, abs_path2, NULL, MS_REMOUNT | MS_BIND, NULL);
                if (r < 0)
                {
                    SYSERROR("error re-mounting %s readwrite", abs_path2);
                    goto out_error;
                }
            }
        }
        else
        {
            r = mkdir_p(abs_path2, 0755);
            if (r < 0 && errno != EEXIST)
            {
                SYSERROR("could not create cgroup directory /sys/fs/cgroup/%s%s", dirname, info->cgroup_path);
                goto out_error;
            }
            if (type == LXC_AUTO_CGROUP_MIXED || type == LXC_AUTO_CGROUP_RO)
            {
                r = mount(abs_path, abs_path, NULL, MS_BIND, NULL);
                if (r < 0)
                {
                    SYSERROR("error bind-mounting %s onto itself", abs_path);
                    goto out_error;
                }
                r = mount(NULL, abs_path, NULL, MS_REMOUNT | MS_BIND | MS_RDONLY, NULL);
                if (r < 0)
                {
                    SYSERROR("error re-mounting %s readonly", abs_path);
                    goto out_error;
                }
            }
            free(abs_path);
            abs_path = NULL;
            abs_path = cgroup_to_absolute_path(mp, info->cgroup_path, NULL);
            if (!abs_path)
                goto out_error;
            r = mount(abs_path, abs_path2, "none", MS_BIND, 0);
            if (r < 0)
            {
                SYSERROR("error bind-mounting %s to %s", abs_path, abs_path2);
                goto out_error;
            }
            if (type == LXC_AUTO_CGROUP_RO)
            {
                r = mount(NULL, abs_path2, NULL, MS_REMOUNT | MS_BIND | MS_RDONLY, NULL);
                if (r < 0)
                {
                    SYSERROR("error re-mounting %s readonly", abs_path2);
                    goto out_error;
                }
            }
        }
        free(abs_path);
        free(abs_path2);
        abs_path = NULL;
        abs_path2 = NULL;
        if (subsystem_count > 1)
        {
            for (i = 0; i < subsystem_count; i++)
            {
                abs_path = lxc_append_paths(path, parts[i]);
                if (!abs_path)
                    goto out_error;
                r = symlink(dirname, abs_path);
                if (r < 0)
                    WARN("could not create symlink %s -> %s in /sys/fs/cgroup of container", parts[i], dirname);
                free(abs_path);
                abs_path = NULL;
            }
        }
        free(dirname);
        free(parts);
        dirname = NULL;
        parts = NULL;
    }
    free(path);
    return true;
out_error:
    saved_errno = errno;
    free(path);
    free(dirname);
    free(parts);
    free(abs_path);
    free(abs_path2);
    errno = saved_errno;
    return false;
}