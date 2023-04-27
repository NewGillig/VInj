int mdb_search(Operation *op, SlapReply *rs)
{
    struct mdb_info *mdb = (struct mdb_info *)op->o_bd->be_private;
    ID id, cursor, nsubs, ncand, cscope;
    ID lastid = NOID;
    ID candidates[MDB_IDL_UM_SIZE];
    ID iscopes[MDB_IDL_DB_SIZE];
    ID2 *scopes;
    void *stack;
    Entry *e = NULL, *base = NULL;
    Entry *matched = NULL;
    AttributeName *attrs;
    slap_mask_t mask;
    time_t stoptime;
    int manageDSAit;
    int tentries = 0;
    IdScopes isc;
    MDB_cursor *mci, *mcd;
    ww_ctx wwctx;
    slap_callback cb = {0};
    mdb_op_info opinfo = {{{0}}}, *moi = &opinfo;
    MDB_txn *ltid = NULL;
    Debug(LDAP_DEBUG_TRACE, "=> " LDAP_XSTRING(mdb_search) "\n", 0, 0, 0);
    attrs = op->oq_search.rs_attrs;
    manageDSAit = get_manageDSAit(op);
    rs->sr_err = mdb_opinfo_get(op, mdb, 1, &moi);
    switch (rs->sr_err)
    {
    case 0:
        break;
    default:
        send_ldap_error(op, rs, LDAP_OTHER, "internal error");
        return rs->sr_err;
    }
    ltid = moi->moi_txn;
    rs->sr_err = mdb_cursor_open(ltid, mdb->mi_id2entry, &mci);
    if (rs->sr_err)
    {
        send_ldap_error(op, rs, LDAP_OTHER, "internal error");
        return rs->sr_err;
    }
    rs->sr_err = mdb_cursor_open(ltid, mdb->mi_dn2id, &mcd);
    if (rs->sr_err)
    {
        mdb_cursor_close(mci);
        send_ldap_error(op, rs, LDAP_OTHER, "internal error");
        return rs->sr_err;
    }
    scopes = scope_chunk_get(op);
    stack = search_stack(op);
    isc.mt = ltid;
    isc.mc = mcd;
    isc.scopes = scopes;
    isc.oscope = op->ors_scope;
    isc.sctmp = stack;
    if (op->ors_deref & LDAP_DEREF_FINDING)
    {
        MDB_IDL_ZERO(candidates);
    }
dn2entry_retry:
    rs->sr_err = mdb_dn2entry(op, ltid, mcd, &op->o_req_ndn, &e, &nsubs, 1);
    switch (rs->sr_err)
    {
    case MDB_NOTFOUND:
        matched = e;
        e = NULL;
        break;
    case 0:
        break;
    case LDAP_BUSY:
        send_ldap_error(op, rs, LDAP_BUSY, "ldap server busy");
        goto done;
    default:
        send_ldap_error(op, rs, LDAP_OTHER, "internal error");
        goto done;
    }
    if (op->ors_deref & LDAP_DEREF_FINDING)
    {
        if (matched && is_entry_alias(matched))
        {
            struct berval stub;
            stub.bv_val = op->o_req_ndn.bv_val;
            stub.bv_len = op->o_req_ndn.bv_len - matched->e_nname.bv_len - 1;
            e = deref_base(op, rs, matched, &matched, ltid, candidates, NULL);
            if (e)
            {
                build_new_dn(&op->o_req_ndn, &e->e_nname, &stub, op->o_tmpmemctx);
                mdb_entry_return(op, e);
                matched = NULL;
                goto dn2entry_retry;
            }
        }
        else if (e && is_entry_alias(e))
        {
            e = deref_base(op, rs, e, &matched, ltid, candidates, NULL);
        }
    }
    if (e == NULL)
    {
        struct berval matched_dn = BER_BVNULL;
        if (matched != NULL)
        {
            BerVarray erefs = NULL;
            if (!access_allowed(op, matched, slap_schema.si_ad_entry, NULL, ACL_DISCLOSE, NULL))
            {
                rs->sr_err = LDAP_NO_SUCH_OBJECT;
            }
            else
            {
                ber_dupbv(&matched_dn, &matched->e_name);
                erefs = is_entry_referral(matched) ? get_entry_referrals(op, matched) : NULL;
                if (rs->sr_err == MDB_NOTFOUND)
                    rs->sr_err = LDAP_REFERRAL;
                rs->sr_matched = matched_dn.bv_val;
            }
            mdb_entry_return(op, matched);
            matched = NULL;
            if (erefs)
            {
                rs->sr_ref = referral_rewrite(erefs, &matched_dn, &op->o_req_dn, op->oq_search.rs_scope);
                ber_bvarray_free(erefs);
            }
        }
        else
        {
            rs->sr_ref = referral_rewrite(default_referral, NULL, &op->o_req_dn, op->oq_search.rs_scope);
            rs->sr_err = rs->sr_ref != NULL ? LDAP_REFERRAL : LDAP_NO_SUCH_OBJECT;
        }
        send_ldap_result(op, rs);
        if (rs->sr_ref)
        {
            ber_bvarray_free(rs->sr_ref);
            rs->sr_ref = NULL;
        }
        if (!BER_BVISNULL(&matched_dn))
        {
            ber_memfree(matched_dn.bv_val);
            rs->sr_matched = NULL;
        }
        goto done;
    }
    if (!access_allowed_mask(op, e, slap_schema.si_ad_entry, NULL, ACL_SEARCH, NULL, &mask))
    {
        if (!ACL_GRANT(mask, ACL_DISCLOSE))
        {
            rs->sr_err = LDAP_NO_SUCH_OBJECT;
        }
        else
        {
            rs->sr_err = LDAP_INSUFFICIENT_ACCESS;
        }
        mdb_entry_return(op, e);
        send_ldap_result(op, rs);
        goto done;
    }
    if (!manageDSAit && is_entry_referral(e))
    {
        struct berval matched_dn = BER_BVNULL;
        BerVarray erefs = NULL;
        ber_dupbv(&matched_dn, &e->e_name);
        erefs = get_entry_referrals(op, e);
        rs->sr_err = LDAP_REFERRAL;
        mdb_entry_return(op, e);
        e = NULL;
        if (erefs)
        {
            rs->sr_ref = referral_rewrite(erefs, &matched_dn, &op->o_req_dn, op->oq_search.rs_scope);
            ber_bvarray_free(erefs);
            if (!rs->sr_ref)
            {
                rs->sr_text = "bad_referral object";
            }
        }
        Debug(LDAP_DEBUG_TRACE, LDAP_XSTRING(mdb_search) ": entry is referral\n", 0, 0, 0);
        rs->sr_matched = matched_dn.bv_val;
        send_ldap_result(op, rs);
        ber_bvarray_free(rs->sr_ref);
        rs->sr_ref = NULL;
        ber_memfree(matched_dn.bv_val);
        rs->sr_matched = NULL;
        goto done;
    }
    if (get_assert(op) && (test_filter(op, e, get_assertion(op)) != LDAP_COMPARE_TRUE))
    {
        rs->sr_err = LDAP_ASSERTION_FAILED;
        mdb_entry_return(op, e);
        send_ldap_result(op, rs);
        goto done;
    }
    stoptime = op->o_time + op->ors_tlimit;
    base = e;
    e = NULL;
    if (op->oq_search.rs_scope == LDAP_SCOPE_BASE)
    {
        rs->sr_err = base_candidate(op->o_bd, base, candidates);
        scopes[0].mid = 0;
        ncand = 1;
    }
    else
    {
        if (op->ors_scope == LDAP_SCOPE_ONELEVEL)
        {
            size_t nkids;
            MDB_val key, data;
            key.mv_data = &base->e_id;
            key.mv_size = sizeof(ID);
            mdb_cursor_get(mcd, &key, &data, MDB_SET);
            mdb_cursor_count(mcd, &nkids);
            nsubs = nkids - 1;
        }
        else if (!base->e_id)
        {
            MDB_stat ms;
            mdb_stat(ltid, mdb->mi_id2entry, &ms);
            nsubs = ms.ms_entries;
        }
        MDB_IDL_ZERO(candidates);
        scopes[0].mid = 1;
        scopes[1].mid = base->e_id;
        scopes[1].mval.mv_data = NULL;
        rs->sr_err = search_candidates(op, rs, base, &isc, mci, candidates, stack);
        ncand = MDB_IDL_N(candidates);
        if (!base->e_id || ncand == NOID)
        {
            MDB_stat ms;
            mdb_stat(ltid, mdb->mi_id2entry, &ms);
            if (!base->e_id)
                nsubs = ms.ms_entries;
            if (ncand == NOID)
                ncand = ms.ms_entries;
        }
    }
    cursor = 0;
    if (candidates[0] == 0)
    {
        Debug(LDAP_DEBUG_TRACE, LDAP_XSTRING(mdb_search) ": no candidates\n", 0, 0, 0);
        goto nochange;
    }
    if (op->ors_limit && op->ors_limit->lms_s_unchecked != -1 && ncand > (unsigned)op->ors_limit->lms_s_unchecked)
    {
        rs->sr_err = LDAP_ADMINLIMIT_EXCEEDED;
        send_ldap_result(op, rs);
        rs->sr_err = LDAP_SUCCESS;
        goto done;
    }
    if (op->ors_limit == NULL || !op->ors_limit->lms_s_pr_hide)
    {
        tentries = ncand;
    }
    wwctx.flag = 0;
    if (moi == &opinfo)
    {
        cb.sc_writewait = mdb_writewait;
        cb.sc_private = &wwctx;
        wwctx.txn = ltid;
        wwctx.mcd = NULL;
        cb.sc_next = op->o_callback;
        op->o_callback = &cb;
    }
    if (get_pagedresults(op) > SLAP_CONTROL_IGNORED)
    {
        PagedResultsState *ps = op->o_pagedresults_state;
        rs->sr_err = parse_paged_cookie(op, rs);
        if (rs->sr_err != LDAP_SUCCESS)
        {
            send_ldap_result(op, rs);
            goto done;
        }
        cursor = (ID)ps->ps_cookie;
        if (cursor && ps->ps_size == 0)
        {
            rs->sr_err = LDAP_SUCCESS;
            rs->sr_text = "search abandoned by pagedResult size=0";
            send_ldap_result(op, rs);
            goto done;
        }
        id = mdb_idl_first(candidates, &cursor);
        if (id == NOID)
        {
            Debug(LDAP_DEBUG_TRACE, LDAP_XSTRING(mdb_search) ": no paged results candidates\n", 0, 0, 0);
            send_paged_response(op, rs, &lastid, 0);
            rs->sr_err = LDAP_OTHER;
            goto done;
        }
        if (id == (ID)ps->ps_cookie)
            id = mdb_idl_next(candidates, &cursor);
        nsubs = ncand;
        goto loop_begin;
    }
    if (nsubs < ncand)
    {
        int rc;
        if (scopes[0].mid > 1)
        {
            cursor = 1;
            for (cscope = 1; cscope <= scopes[0].mid; cscope++)
            {
                if (scopes[cscope].mid == base->e_id)
                    continue;
                iscopes[cursor++] = scopes[cscope].mid;
            }
            iscopes[0] = scopes[0].mid - 1;
        }
        else
        {
            iscopes[0] = 0;
        }
        wwctx.mcd = mcd;
        isc.id = base->e_id;
        isc.numrdns = 0;
        rc = mdb_dn2id_walk(op, &isc);
        if (rc)
            id = NOID;
        else
            id = isc.id;
        cscope = 0;
    }
    else
    {
        id = mdb_idl_first(candidates, &cursor);
    }
    while (id != NOID)
    {
        int scopeok;
        MDB_val edata;
    loop_begin:
        if (op->o_abandon)
        {
            rs->sr_err = SLAPD_ABANDON;
            send_ldap_result(op, rs);
            goto done;
        }
        if (slapd_shutdown)
        {
            rs->sr_err = LDAP_UNAVAILABLE;
            send_ldap_disconnect(op, rs);
            goto done;
        }
        if (op->ors_tlimit != SLAP_NO_LIMIT && slap_get_time() > stoptime)
        {
            rs->sr_err = LDAP_TIMELIMIT_EXCEEDED;
            rs->sr_ref = rs->sr_v2ref;
            send_ldap_result(op, rs);
            rs->sr_err = LDAP_SUCCESS;
            goto done;
        }
        if (nsubs < ncand)
        {
            unsigned i;
            scopeok = 0;
            if (MDB_IDL_IS_RANGE(candidates))
            {
                if (id >= MDB_IDL_RANGE_FIRST(candidates) && id <= MDB_IDL_RANGE_LAST(candidates))
                    scopeok = 1;
            }
            else
            {
                i = mdb_idl_search(candidates, id);
                if (i <= candidates[0] && candidates[i] == id)
                    scopeok = 1;
            }
            if (scopeok)
                goto scopeok;
            goto loop_continue;
        }
        scopeok = 0;
        isc.numrdns = 0;
        switch (op->ors_scope)
        {
        case LDAP_SCOPE_BASE:
            if (id == base->e_id)
                scopeok = 1;
            break;
            scopeok = 1;
            break;
        }
    case LDAP_SCOPE_ONELEVEL:
        if (id == base->e_id)
            break;
        isc.id = id;
        isc.nscope = 0;
        rs->sr_err = mdb_idscopes(op, &isc);
        if (rs->sr_err == MDB_SUCCESS)
        {
            if (isc.nscope)
                scopeok = 1;
        }
        else
        {
            if (rs->sr_err == MDB_NOTFOUND)
                goto notfound;
        }
        break;
    }
    if (!scopeok)
    {
        Debug(LDAP_DEBUG_TRACE, LDAP_XSTRING(mdb_search) ": %ld scope not okay\n", (long)id, 0, 0);
        goto loop_continue;
    }
scopeok:
    if (id == base->e_id)
    {
        e = base;
    }
    else
    {
        rs->sr_err = mdb_id2edata(op, mci, id, &edata);
        if (rs->sr_err == MDB_NOTFOUND)
        {
        notfound:
            if (nsubs < ncand)
                goto loop_continue;
            if (!MDB_IDL_IS_RANGE(candidates))
            {
                Debug(LDAP_DEBUG_TRACE, LDAP_XSTRING(mdb_search) ": candidate %ld not found\n", (long)id, 0, 0);
            }
            else
            {
                rs->sr_err = mdb_get_nextid(mci, &cursor);
                if (rs->sr_err == MDB_NOTFOUND)
                {
                    break;
                }
                if (rs->sr_err)
                {
                    rs->sr_err = LDAP_OTHER;
                    rs->sr_text = "internal error in get_nextid";
                    send_ldap_result(op, rs);
                    goto done;
                }
                cursor--;
            }
            goto loop_continue;
        }
        else if (rs->sr_err)
        {
            rs->sr_err = LDAP_OTHER;
            rs->sr_text = "internal error in mdb_id2edata";
            send_ldap_result(op, rs);
            goto done;
        }
        rs->sr_err = mdb_entry_decode(op, ltid, &edata, id, &e);
        if (rs->sr_err)
        {
            rs->sr_err = LDAP_OTHER;
            rs->sr_text = "internal error in mdb_entry_decode";
            send_ldap_result(op, rs);
            goto done;
        }
        e->e_id = id;
        e->e_name.bv_val = NULL;
        e->e_nname.bv_val = NULL;
    }
    if (is_entry_subentry(e))
    {
        if (op->oq_search.rs_scope != LDAP_SCOPE_BASE)
        {
            if (!get_subentries_visibility(op))
            {
                goto loop_continue;
            }
        }
        else if (get_subentries(op) && !get_subentries_visibility(op))
        {
            goto loop_continue;
        }
    }
    else if (get_subentries_visibility(op))
    {
        goto loop_continue;
    }
    if (op->ors_deref & LDAP_DEREF_SEARCHING)
    {
        if (is_entry_alias(e) && ((op->ors_deref & LDAP_DEREF_FINDING) || e != base))
        {
            goto loop_continue;
        }
    }
    if (!manageDSAit && is_entry_glue(e))
    {
        goto loop_continue;
    }
    if (e != base)
    {
        struct berval pdn, pndn;
        char *d, *n;
        int i;
        if (nsubs < ncand || isc.scopes[isc.nscope].mid == base->e_id)
        {
            pdn = base->e_name;
            pndn = base->e_nname;
        }
        else
        {
            mdb_id2name(op, ltid, &isc.mc, scopes[isc.nscope].mid, &pdn, &pndn);
        }
        e->e_name.bv_len = pdn.bv_len;
        e->e_nname.bv_len = pndn.bv_len;
        for (i = 0; i < isc.numrdns; i++)
        {
            e->e_name.bv_len += isc.rdns[i].bv_len + 1;
            e->e_nname.bv_len += isc.nrdns[i].bv_len + 1;
        }
        e->e_name.bv_val = op->o_tmpalloc(e->e_name.bv_len + 1, op->o_tmpmemctx);
        e->e_nname.bv_val = op->o_tmpalloc(e->e_nname.bv_len + 1, op->o_tmpmemctx);
        d = e->e_name.bv_val;
        n = e->e_nname.bv_val;
        if (nsubs < ncand)
        {
            for (i = isc.numrdns - 1; i >= 0; i--)
            {
                memcpy(d, isc.rdns[i].bv_val, isc.rdns[i].bv_len);
                d += isc.rdns[i].bv_len;
                *d++ = ',';
                memcpy(n, isc.nrdns[i].bv_val, isc.nrdns[i].bv_len);
                n += isc.nrdns[i].bv_len;
                *n++ = ',';
            }
        }
        else
        {
            for (i = 0; i < isc.numrdns; i++)
            {
                memcpy(d, isc.rdns[i].bv_val, isc.rdns[i].bv_len);
                d += isc.rdns[i].bv_len;
                *d++ = ',';
                memcpy(n, isc.nrdns[i].bv_val, isc.nrdns[i].bv_len);
                n += isc.nrdns[i].bv_len;
                *n++ = ',';
            }
        }
        if (pdn.bv_len)
        {
            memcpy(d, pdn.bv_val, pdn.bv_len + 1);
            memcpy(n, pndn.bv_val, pndn.bv_len + 1);
        }
        else
        {
            *--d = '\0';
            *--n = '\0';
            e->e_name.bv_len--;
            e->e_nname.bv_len--;
        }
        if (pndn.bv_val != base->e_nname.bv_val)
        {
            op->o_tmpfree(pndn.bv_val, op->o_tmpmemctx);
            op->o_tmpfree(pdn.bv_val, op->o_tmpmemctx);
        }
    }
    if (!manageDSAit && op->oq_search.rs_scope != LDAP_SCOPE_BASE && is_entry_referral(e))
    {
        BerVarray erefs = get_entry_referrals(op, e);
        rs->sr_ref = referral_rewrite(erefs, &e->e_name, NULL, op->oq_search.rs_scope == LDAP_SCOPE_ONELEVEL ? LDAP_SCOPE_BASE : LDAP_SCOPE_SUBTREE);
        rs->sr_entry = e;
        rs->sr_flags = 0;
        send_search_reference(op, rs);
        if (e != base)
            mdb_entry_return(op, e);
        rs->sr_entry = NULL;
        e = NULL;
        ber_bvarray_free(rs->sr_ref);
        ber_bvarray_free(erefs);
        rs->sr_ref = NULL;
        goto loop_continue;
    }
    rs->sr_err = test_filter(op, e, op->oq_search.rs_filter);
    if (rs->sr_err == LDAP_COMPARE_TRUE)
    {
        if (get_pagedresults(op) > SLAP_CONTROL_IGNORED)
        {
            if (rs->sr_nentries >= ((PagedResultsState *)op->o_pagedresults_state)->ps_size)
            {
                if (e != base)
                    mdb_entry_return(op, e);
                e = NULL;
                send_paged_response(op, rs, &lastid, tentries);
                goto done;
            }
            lastid = id;
        }
        if (e)
        {
            rs->sr_attrs = op->oq_search.rs_attrs;
            rs->sr_operational_attrs = NULL;
            rs->sr_ctrls = NULL;
            rs->sr_entry = e;
            RS_ASSERT(e->e_private != NULL);
            rs->sr_flags = 0;
            rs->sr_err = LDAP_SUCCESS;
            rs->sr_err = send_search_entry(op, rs);
            rs->sr_attrs = NULL;
            rs->sr_entry = NULL;
            if (e != base)
                mdb_entry_return(op, e);
            e = NULL;
            switch (rs->sr_err)
            {
            case LDAP_SUCCESS:
                break;
            default:
                break;
            case LDAP_BUSY:
                send_ldap_result(op, rs);
                goto done;
            case LDAP_UNAVAILABLE:
            case LDAP_SIZELIMIT_EXCEEDED:
                if (rs->sr_err == LDAP_SIZELIMIT_EXCEEDED)
                {
                    rs->sr_ref = rs->sr_v2ref;
                    send_ldap_result(op, rs);
                    rs->sr_err = LDAP_SUCCESS;
                }
                else
                {
                    rs->sr_err = LDAP_OTHER;
                }
                goto done;
            }
        }
    }
    else
    {
        Debug(LDAP_DEBUG_TRACE, LDAP_XSTRING(mdb_search) ": %ld does not match filter\n", (long)id, 0, 0);
    }
loop_continue:
    if (moi == &opinfo && !wwctx.flag && mdb->mi_rtxn_size)
    {
        wwctx.nentries++;
        if (wwctx.nentries >= mdb->mi_rtxn_size)
        {
            wwctx.nentries = 0;
            mdb_rtxn_snap(op, &wwctx);
        }
    }
    if (wwctx.flag)
    {
        rs->sr_err = mdb_waitfixup(op, &wwctx, mci, mcd, &isc);
        if (rs->sr_err)
        {
            send_ldap_result(op, rs);
            goto done;
        }
    }
    if (e != NULL)
    {
        if (e != base)
            mdb_entry_return(op, e);
        RS_ASSERT(rs->sr_entry == NULL);
        e = NULL;
        rs->sr_entry = NULL;
    }
    if (nsubs < ncand)
    {
        int rc = mdb_dn2id_walk(op, &isc);
        if (rc)
        {
            id = NOID;
            while (iscopes[0] && cscope < iscopes[0])
            {
                cscope++;
                isc.id = iscopes[cscope];
                if (base)
                    mdb_entry_return(op, base);
                rs->sr_err = mdb_id2entry(op, mci, isc.id, &base);
                if (!rs->sr_err)
                {
                    mdb_id2name(op, ltid, &isc.mc, isc.id, &base->e_name, &base->e_nname);
                    isc.numrdns = 0;
                    if (isc.oscope == LDAP_SCOPE_ONELEVEL)
                        isc.oscope = LDAP_SCOPE_BASE;
                    rc = mdb_dn2id_walk(op, &isc);
                    if (!rc)
                    {
                        id = isc.id;
                        break;
                    }
                }
            }
        }
        else
            id = isc.id;
    }
    else
    {
        id = mdb_idl_next(candidates, &cursor);
    }
}