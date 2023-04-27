static void _slurm_rpc_allocate_pack(slurm_msg_t *msg)
{
    static int select_serial = -1;
    static int active_rpc_cnt = 0;
    int error_code = SLURM_SUCCESS, inx, pack_cnt = -1;
    DEF_TIMERS;
    job_desc_msg_t *job_desc_msg;
    List job_req_list = (List)msg->data;
    slurmctld_lock_t job_write_lock = {READ_LOCK, WRITE_LOCK, WRITE_LOCK, READ_LOCK, READ_LOCK};
    uid_t uid = g_slurm_auth_get_uid(msg->auth_cred, slurmctld_config.auth_info);
    uint32_t job_uid = NO_VAL;
    struct job_record *job_ptr, *first_job_ptr = NULL;
    char *err_msg = NULL, **job_submit_user_msg = NULL;
    ListIterator iter;
    bool priv_user;
    List submit_job_list = NULL;
    uint32_t pack_job_id = 0, pack_job_offset = 0;
    hostset_t jobid_hostset = NULL;
    char tmp_str[32];
    List resp = NULL;
    slurm_addr_t resp_addr;
    char resp_host[16];
    uint16_t port;
    START_TIMER;
    if (select_serial == -1)
    {
        if (xstrcmp(slurmctld_conf.select_type, "select/serial"))
            select_serial = 0;
        else
            select_serial = 1;
    }
    if (slurmctld_config.submissions_disabled || (select_serial == 1))
    {
        info("Submissions disabled on system");
        error_code = ESLURM_SUBMISSIONS_DISABLED;
        goto send_msg;
    }
    if (!_sched_backfill())
    {
        error_code = ESLURM_NOT_SUPPORTED;
        goto send_msg;
    }
    if (!job_req_list || (list_count(job_req_list) == 0))
    {
        info("REQUEST_JOB_PACK_ALLOCATION from uid=%d with empty job list", uid);
        error_code = SLURM_ERROR;
        goto send_msg;
    }
    if (slurm_get_peer_addr(msg->conn_fd, &resp_addr) == 0)
    {
        slurm_get_ip_str(&resp_addr, &port, resp_host, sizeof(resp_host));
    }
    else
    {
        info("REQUEST_JOB_PACK_ALLOCATION from uid=%d , can't get peer addr", uid);
        error_code = SLURM_ERROR;
        goto send_msg;
    }
    debug2("sched: Processing RPC: REQUEST_JOB_PACK_ALLOCATION from uid=%d", uid);
    pack_cnt = list_count(job_req_list);
    job_submit_user_msg = xmalloc(sizeof(char *) * pack_cnt);
    priv_user = validate_slurm_user(uid);
    submit_job_list = list_create(NULL);
    _throttle_start(&active_rpc_cnt);
    lock_slurmctld(job_write_lock);
    inx = 0;
    iter = list_iterator_create(job_req_list);
    while ((job_desc_msg = (job_desc_msg_t *)list_next(iter)))
    {
        if (job_uid == NO_VAL)
            job_uid = job_desc_msg->user_id;
        if ((uid != job_desc_msg->user_id) && !priv_user)
        {
            error_code = ESLURM_USER_ID_MISSING;
            error("Security violation, REQUEST_JOB_PACK_ALLOCATION from uid=%d", uid);
            break;
        }
        if ((job_desc_msg->alloc_node == NULL) || (job_desc_msg->alloc_node[0] == '\0'))
        {
            error_code = ESLURM_INVALID_NODE_NAME;
            error("REQUEST_JOB_PACK_ALLOCATION lacks alloc_node from uid=%d", uid);
            break;
        }
        if (job_desc_msg->array_inx)
        {
            error_code = ESLURM_INVALID_ARRAY;
            break;
        }
        if (job_desc_msg->immediate)
        {
            error_code = ESLURM_CAN_NOT_START_IMMEDIATELY;
            break;
        }
        job_desc_msg->pack_job_offset = pack_job_offset;
        error_code = validate_job_create_req(job_desc_msg, uid, &job_submit_user_msg[inx++]);
        if (error_code)
            break;
        error_code = ESLURM_RESERVATION_BUSY;
        error("attempt to nest ALPS allocation on %s:%d by uid=%d", job_desc_msg->alloc_node, job_desc_msg->alloc_sid, uid);
        break;
    }
    job_ptr = NULL;
    if (!job_desc_msg->resp_host)
        job_desc_msg->resp_host = xstrdup(resp_host);
    if (pack_job_offset)
    {
        job_desc_msg->mail_type = 0;
        xfree(job_desc_msg->mail_user);
    }
    job_desc_msg->pack_job_offset = pack_job_offset;
    error_code = job_allocate(job_desc_msg, false, false, NULL, true, uid, &job_ptr, &err_msg, msg->protocol_version);
    if (!job_ptr)
    {
        if (error_code == SLURM_SUCCESS)
            error_code = SLURM_ERROR;
        break;
    }
    if (error_code && (job_ptr->job_state == JOB_FAILED))
        break;
    error_code = SLURM_SUCCESS;
    if (pack_job_id == 0)
    {
        pack_job_id = job_ptr->job_id;
        first_job_ptr = job_ptr;
    }
    snprintf(tmp_str, sizeof(tmp_str), "%u", job_ptr->job_id);
    if (jobid_hostset)
        hostset_insert(jobid_hostset, tmp_str);
    else
        jobid_hostset = hostset_create(tmp_str);
    job_ptr->pack_job_id = pack_job_id;
    job_ptr->pack_job_offset = pack_job_offset++;
    list_append(submit_job_list, job_ptr);
}