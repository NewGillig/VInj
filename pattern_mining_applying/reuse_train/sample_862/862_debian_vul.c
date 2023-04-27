extern List as_mysql_modify_job(mysql_conn_t *mysql_conn, uint32_t uid, slurmdb_job_modify_cond_t *job_cond, slurmdb_job_rec_t *job)
{
    List ret_list = NULL;
    int rc = SLURM_SUCCESS;
    char *object = NULL;
    char *vals = NULL, *query = NULL, *cond_char = NULL;
    time_t now = time(NULL);
    char *user_name = NULL;
    MYSQL_RES *result = NULL;
    MYSQL_ROW row;
    if (!job_cond || !job)
    {
        error("we need something to change");
        return NULL;
    }
    else if (job_cond->job_id == NO_VAL)
    {
        errno = SLURM_NO_CHANGE_IN_DATA;
        error("Job ID was not specified for job modification\n");
        return NULL;
    }
    else if (!job_cond->cluster)
    {
        errno = SLURM_NO_CHANGE_IN_DATA;
        error("Cluster was not specified for job modification\n");
        return NULL;
    }
    else if (check_connection(mysql_conn) != SLURM_SUCCESS)
        return NULL;
    if (job->derived_ec != NO_VAL)
        xstrfmtcat(vals, ", derived_ec=%u", job->derived_ec);
    if (job->derived_es)
    {
        char *derived_es = slurm_add_slash_to_quotes(job->derived_es);
        xstrfmtcat(vals, ", derived_es='%s'", derived_es);
        xfree(derived_es);
    }
    if (!vals)
    {
        errno = SLURM_NO_CHANGE_IN_DATA;
        error("No change specified for job modification");
        return NULL;
    }
    query = xstrdup_printf("select job_db_inx, id_job, time_submit, "
                           "id_user "
                           "from \"%s_%s\" where deleted=0 "
                           "&& id_job=%u "
                           "order by time_submit desc limit 1; ",
                           job_cond->cluster, job_table, job_cond->job_id);
    if (debug_flags & DEBUG_FLAG_DB_JOB)
        DB_DEBUG(mysql_conn->conn, "query\n%s", query);
    if (!(result = mysql_db_query_ret(mysql_conn, query, 0)))
    {
        xfree(vals);
        xfree(query);
        return NULL;
    }
    if ((row = mysql_fetch_row(result)))
    {
        char tmp_char[25];
        time_t time_submit = atol(row[2]);
        if ((uid != atoi(row[3])) && !is_user_min_admin_level(mysql_conn, uid, SLURMDB_ADMIN_OPERATOR))
        {
            errno = ESLURM_ACCESS_DENIED;
            xfree(vals);
            xfree(query);
            mysql_free_result(result);
            return NULL;
        }
        slurm_make_time_str(&time_submit, tmp_char, sizeof(tmp_char));
        xstrfmtcat(cond_char, "job_db_inx=%s", row[0]);
        object = xstrdup_printf("%s submitted at %s", row[1], tmp_char);
        ret_list = list_create(slurm_destroy_char);
        list_append(ret_list, object);
        mysql_free_result(result);
    }
    else
    {
        errno = ESLURM_INVALID_JOB_ID;
        if (debug_flags & DEBUG_FLAG_DB_JOB)
            DB_DEBUG(mysql_conn->conn, "as_mysql_modify_job: Job not found\n%s", query);
        xfree(vals);
        xfree(query);
        mysql_free_result(result);
        return NULL;
    }
    xfree(query);
    user_name = uid_to_string((uid_t)uid);
    rc = modify_common(mysql_conn, DBD_MODIFY_JOB, now, user_name, job_table, cond_char, vals, job_cond->cluster);
    xfree(user_name);
    xfree(cond_char);
    xfree(vals);
    if (rc == SLURM_ERROR)
    {
        error("Couldn't modify job");
        FREE_NULL_LIST(ret_list);
        ret_list = NULL;
    }
    return ret_list;
}