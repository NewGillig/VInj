void *merge_directory_configs(apr_pool_t *mp, void *_parent, void *_child)
{
    directory_config *parent = (directory_config *)_parent;
    directory_config *child = (directory_config *)_child;
    directory_config *merged = create_directory_config(mp, NULL);
    merged->is_enabled = (child->is_enabled == NOT_SET ? parent->is_enabled : child->is_enabled);
    merged->reqbody_access = (child->reqbody_access == NOT_SET ? parent->reqbody_access : child->reqbody_access);
    merged->reqbody_buffering = (child->reqbody_buffering == NOT_SET ? parent->reqbody_buffering : child->reqbody_buffering);
    merged->reqbody_inmemory_limit = (child->reqbody_inmemory_limit == NOT_SET ? parent->reqbody_inmemory_limit : child->reqbody_inmemory_limit);
    merged->reqbody_limit = (child->reqbody_limit == NOT_SET ? parent->reqbody_limit : child->reqbody_limit);
    merged->reqbody_no_files_limit = (child->reqbody_no_files_limit == NOT_SET ? parent->reqbody_no_files_limit : child->reqbody_no_files_limit);
    merged->resbody_access = (child->resbody_access == NOT_SET ? parent->resbody_access : child->resbody_access);
    merged->of_limit = (child->of_limit == NOT_SET ? parent->of_limit : child->of_limit);
    merged->if_limit_action = (child->if_limit_action == NOT_SET ? parent->if_limit_action : child->if_limit_action);
    merged->of_limit_action = (child->of_limit_action == NOT_SET ? parent->of_limit_action : child->of_limit_action);
    merged->reqintercept_oe = (child->reqintercept_oe == NOT_SET ? parent->reqintercept_oe : child->reqintercept_oe);
    if (child->of_mime_types != NOT_SET_P)
    {
        if (child->of_mime_types_cleared == 1)
        {
            merged->of_mime_types = child->of_mime_types;
            merged->of_mime_types_cleared = 1;
        }
        else
        {
            if (parent->of_mime_types == NOT_SET_P)
            {
                merged->of_mime_types = child->of_mime_types;
                merged->of_mime_types_cleared = NOT_SET;
            }
            else
            {
                merged->of_mime_types = apr_table_overlay(mp, parent->of_mime_types, child->of_mime_types);
                if (merged->of_mime_types == NULL)
                    return NULL;
            }
        }
    }
    else
    {
        if (child->of_mime_types_cleared == 1)
        {
            merged->of_mime_types_cleared = 1;
        }
        else
        {
            merged->of_mime_types = parent->of_mime_types;
            merged->of_mime_types_cleared = parent->of_mime_types_cleared;
        }
    }
    if (child->debuglog_fd == NOT_SET_P)
    {
        merged->debuglog_name = parent->debuglog_name;
        merged->debuglog_fd = parent->debuglog_fd;
    }
    else
    {
        merged->debuglog_name = child->debuglog_name;
        merged->debuglog_fd = child->debuglog_fd;
    }
    merged->debuglog_level = (child->debuglog_level == NOT_SET ? parent->debuglog_level : child->debuglog_level);
    merged->cookie_format = (child->cookie_format == NOT_SET ? parent->cookie_format : child->cookie_format);
    merged->argument_separator = (child->argument_separator == NOT_SET ? parent->argument_separator : child->argument_separator);
    merged->cookiev0_separator = (child->cookiev0_separator == NOT_SET_P ? parent->cookiev0_separator : child->cookiev0_separator);
    if ((child->rule_inheritance == NOT_SET) || (child->rule_inheritance == 1))
    {
        merged->rule_inheritance = parent->rule_inheritance;
        if ((child->ruleset == NULL) && (parent->ruleset == NULL))
        {
            else if (child->ruleset == NULL) { copy_rules(mp, parent->ruleset, merged->ruleset, child->rule_exceptions); }
            else if (parent->ruleset == NULL)
            {
                merged->ruleset->phase_request_headers = apr_array_copy(mp, child->ruleset->phase_request_headers);
                merged->ruleset->phase_request_body = apr_array_copy(mp, child->ruleset->phase_request_body);
                merged->ruleset->phase_response_headers = apr_array_copy(mp, child->ruleset->phase_response_headers);
                merged->ruleset->phase_response_body = apr_array_copy(mp, child->ruleset->phase_response_body);
                merged->ruleset->phase_logging = apr_array_copy(mp, child->ruleset->phase_logging);
            }
            else
            {
                copy_rules(mp, parent->ruleset, merged->ruleset, child->rule_exceptions);
                apr_array_cat(merged->ruleset->phase_request_headers, child->ruleset->phase_request_headers);
                apr_array_cat(merged->ruleset->phase_request_body, child->ruleset->phase_request_body);
                apr_array_cat(merged->ruleset->phase_response_headers, child->ruleset->phase_response_headers);
                apr_array_cat(merged->ruleset->phase_response_body, child->ruleset->phase_response_body);
                apr_array_cat(merged->ruleset->phase_logging, child->ruleset->phase_logging);
            }
        }
        else
        {
            merged->rule_inheritance = 0;
            if (child->ruleset != NULL)
            {
                merged->ruleset = msre_ruleset_create(child->ruleset->engine, mp);
                merged->ruleset->phase_request_headers = apr_array_copy(mp, child->ruleset->phase_request_headers);
                merged->ruleset->phase_request_body = apr_array_copy(mp, child->ruleset->phase_request_body);
                merged->ruleset->phase_response_headers = apr_array_copy(mp, child->ruleset->phase_response_headers);
                merged->ruleset->phase_response_body = apr_array_copy(mp, child->ruleset->phase_response_body);
                merged->ruleset->phase_logging = apr_array_copy(mp, child->ruleset->phase_logging);
            }
        }
        merged->rule_exceptions = apr_array_append(mp, parent->rule_exceptions, child->rule_exceptions);
        merged->hash_method = apr_array_append(mp, parent->hash_method, child->hash_method);
        merged->auditlog_flag = (child->auditlog_flag == NOT_SET ? parent->auditlog_flag : child->auditlog_flag);
        merged->auditlog_type = (child->auditlog_type == NOT_SET ? parent->auditlog_type : child->auditlog_type);
        merged->max_rule_time = (child->max_rule_time == NOT_SET ? parent->max_rule_time : child->max_rule_time);
        merged->auditlog_dirperms = (child->auditlog_dirperms == NOT_SET ? parent->auditlog_dirperms : child->auditlog_dirperms);
        merged->auditlog_fileperms = (child->auditlog_fileperms == NOT_SET ? parent->auditlog_fileperms : child->auditlog_fileperms);
        if (child->auditlog_fd != NOT_SET_P)
        {
            merged->auditlog_fd = child->auditlog_fd;
            merged->auditlog_name = child->auditlog_name;
        }
        else
        {
            merged->auditlog_fd = parent->auditlog_fd;
            merged->auditlog_name = parent->auditlog_name;
        }
        if (child->auditlog2_fd != NOT_SET_P)
        {
            merged->auditlog2_fd = child->auditlog2_fd;
            merged->auditlog2_name = child->auditlog2_name;
        }
        else
        {
            merged->auditlog2_fd = parent->auditlog2_fd;
            merged->auditlog2_name = parent->auditlog2_name;
        }
        merged->auditlog_storage_dir = (child->auditlog_storage_dir == NOT_SET_P ? parent->auditlog_storage_dir : child->auditlog_storage_dir);
        merged->auditlog_parts = (child->auditlog_parts == NOT_SET_P ? parent->auditlog_parts : child->auditlog_parts);
        merged->auditlog_relevant_regex = (child->auditlog_relevant_regex == NOT_SET_P ? parent->auditlog_relevant_regex : child->auditlog_relevant_regex);
        merged->tmp_dir = (child->tmp_dir == NOT_SET_P ? parent->tmp_dir : child->tmp_dir);
        merged->upload_dir = (child->upload_dir == NOT_SET_P ? parent->upload_dir : child->upload_dir);
        merged->upload_keep_files = (child->upload_keep_files == NOT_SET ? parent->upload_keep_files : child->upload_keep_files);
        merged->upload_validates_files = (child->upload_validates_files == NOT_SET ? parent->upload_validates_files : child->upload_validates_files);
        merged->upload_filemode = (child->upload_filemode == NOT_SET ? parent->upload_filemode : child->upload_filemode);
        merged->upload_file_limit = (child->upload_file_limit == NOT_SET ? parent->upload_file_limit : child->upload_file_limit);
        merged->data_dir = (child->data_dir == NOT_SET_P ? parent->data_dir : child->data_dir);
        merged->webappid = (child->webappid == NOT_SET_P ? parent->webappid : child->webappid);
        merged->sensor_id = (child->sensor_id == NOT_SET_P ? parent->sensor_id : child->sensor_id);
        merged->httpBlkey = (child->httpBlkey == NOT_SET_P ? parent->httpBlkey : child->httpBlkey);
        merged->content_injection_enabled = (child->content_injection_enabled == NOT_SET ? parent->content_injection_enabled : child->content_injection_enabled);
        merged->stream_inbody_inspection = (child->stream_inbody_inspection == NOT_SET ? parent->stream_inbody_inspection : child->stream_inbody_inspection);
        merged->stream_outbody_inspection = (child->stream_outbody_inspection == NOT_SET ? parent->stream_outbody_inspection : child->stream_outbody_inspection);
        merged->geo = (child->geo == NOT_SET_P ? parent->geo : child->geo);
        merged->gsb = (child->gsb == NOT_SET_P ? parent->gsb : child->gsb);
        merged->u_map = (child->u_map == NOT_SET_P ? parent->u_map : child->u_map);
        merged->cache_trans = (child->cache_trans == NOT_SET ? parent->cache_trans : child->cache_trans);
        merged->cache_trans_incremental = (child->cache_trans_incremental == NOT_SET ? parent->cache_trans_incremental : child->cache_trans_incremental);
        merged->cache_trans_min = (child->cache_trans_min == (apr_size_t)NOT_SET ? parent->cache_trans_min : child->cache_trans_min);
        merged->cache_trans_max = (child->cache_trans_max == (apr_size_t)NOT_SET ? parent->cache_trans_max : child->cache_trans_max);
        merged->cache_trans_maxitems = (child->cache_trans_maxitems == (apr_size_t)NOT_SET ? parent->cache_trans_maxitems : child->cache_trans_maxitems);
        merged->component_signatures = apr_array_append(mp, parent->component_signatures, child->component_signatures);
        merged->request_encoding = (child->request_encoding == NOT_SET_P ? parent->request_encoding : child->request_encoding);
        merged->disable_backend_compression = (child->disable_backend_compression == NOT_SET ? parent->disable_backend_compression : child->disable_backend_compression);
        merged->col_timeout = (child->col_timeout == NOT_SET ? parent->col_timeout : child->col_timeout);
        merged->crypto_key = (child->crypto_key == NOT_SET_P ? parent->crypto_key : child->crypto_key);
        merged->crypto_key_len = (child->crypto_key_len == NOT_SET ? parent->crypto_key_len : child->crypto_key_len);
        merged->crypto_key_add = (child->crypto_key_add == NOT_SET ? parent->crypto_key_add : child->crypto_key_add);
        merged->crypto_param_name = (child->crypto_param_name == NOT_SET_P ? parent->crypto_param_name : child->crypto_param_name);
        merged->hash_is_enabled = (child->hash_is_enabled == NOT_SET ? parent->hash_is_enabled : child->hash_is_enabled);
        merged->hash_enforcement = (child->hash_enforcement == NOT_SET ? parent->hash_enforcement : child->hash_enforcement);
        merged->crypto_hash_href_rx = (child->crypto_hash_href_rx == NOT_SET ? parent->crypto_hash_href_rx : child->crypto_hash_href_rx);
        merged->crypto_hash_faction_rx = (child->crypto_hash_faction_rx == NOT_SET ? parent->crypto_hash_faction_rx : child->crypto_hash_faction_rx);
        merged->crypto_hash_location_rx = (child->crypto_hash_location_rx == NOT_SET ? parent->crypto_hash_location_rx : child->crypto_hash_location_rx);
        merged->crypto_hash_iframesrc_rx = (child->crypto_hash_iframesrc_rx == NOT_SET ? parent->crypto_hash_iframesrc_rx : child->crypto_hash_iframesrc_rx);
        merged->crypto_hash_framesrc_rx = (child->crypto_hash_framesrc_rx == NOT_SET ? parent->crypto_hash_framesrc_rx : child->crypto_hash_framesrc_rx);
        merged->crypto_hash_href_pm = (child->crypto_hash_href_pm == NOT_SET ? parent->crypto_hash_href_pm : child->crypto_hash_href_pm);
        merged->crypto_hash_faction_pm = (child->crypto_hash_faction_pm == NOT_SET ? parent->crypto_hash_faction_pm : child->crypto_hash_faction_pm);
        merged->crypto_hash_location_pm = (child->crypto_hash_location_pm == NOT_SET ? parent->crypto_hash_location_pm : child->crypto_hash_location_pm);
        merged->crypto_hash_iframesrc_pm = (child->crypto_hash_iframesrc_pm == NOT_SET ? parent->crypto_hash_iframesrc_pm : child->crypto_hash_iframesrc_pm);
        merged->crypto_hash_framesrc_pm = (child->crypto_hash_framesrc_pm == NOT_SET ? parent->crypto_hash_framesrc_pm : child->crypto_hash_framesrc_pm);
        return merged;