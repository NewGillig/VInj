void kadmin_addprinc(int argc, char *argv[])
{
    kadm5_principal_ent_rec princ;
    long mask;
    krb5_boolean randkey = FALSE, old_style_randkey = FALSE;
    int n_ks_tuple;
    krb5_key_salt_tuple *ks_tuple = NULL;
    char *pass, *canon = NULL;
    krb5_error_code retval;
    char newpw[1024], dummybuf[256];
    static char prompt1[1024], prompt2[1024];
    memset(&princ, 0, sizeof(princ));
    princ.attributes = 0;
    if (kadmin_parse_princ_args(argc, argv, &princ, &mask, &pass, &randkey, &ks_tuple, &n_ks_tuple, "add_principal"))
    {
        kadmin_addprinc_usage();
        goto cleanup;
    }
    retval = krb5_unparse_name(context, princ.principal, &canon);
    if (retval)
    {
        com_err("add_principal", retval, _("while canonicalizing principal"));
        goto cleanup;
    }
    if (mask & KADM5_POLICY)
    {
        if (!policy_exists(princ.policy))
        {
            fprintf(stderr, _("WARNING: policy \"%s\" does not exist\n"), princ.policy);
        }
    }
    else if (!(mask & KADM5_POLICY_CLR))
    {
        if (policy_exists("default"))
        {
            fprintf(stderr, _("NOTICE: no policy specified for %s; "
                              " assigning \ " default \ "\n"),
                    canon);
            princ.policy = "default";
            mask |= KADM5_POLICY;
        }
        else
            fprintf(stderr, _("WARNING: no policy specified for %s; "
                              " defaulting to no policy \ n "),
                    canon);
    }
    mask &= ~KADM5_POLICY_CLR;
    if (randkey)
    {
        pass = NULL;
    }
    else if (pass == NULL)
    {
        unsigned int sz = sizeof(newpw) - 1;
        snprintf(prompt1, sizeof(prompt1), _("Enter password for principal \"%s\""), canon);
        snprintf(prompt2, sizeof(prompt2), _("Re-enter password for principal \"%s\""), canon);
        retval = krb5_read_password(context, prompt1, prompt2, newpw, &sz);
        if (retval)
        {
            com_err("add_principal", retval, _("while reading password for \"%s\"."), canon);
            goto cleanup;
        }
        pass = newpw;
    }
    mask |= KADM5_PRINCIPAL;
    retval = create_princ(&princ, mask, n_ks_tuple, ks_tuple, pass);
    if (retval == EINVAL && randkey)
    {
        prepare_dummy_password(dummybuf, sizeof(dummybuf));
        princ.attributes |= KRB5_KDB_DISALLOW_ALL_TIX;
        mask |= KADM5_ATTRIBUTES;
        pass = dummybuf;
        retval = create_princ(&princ, mask, n_ks_tuple, ks_tuple, pass);
        old_style_randkey = 1;
    }
    if (retval)
    {
        com_err("add_principal", retval, "while creating \"%s\".", canon);
        goto cleanup;
    }
    if (old_style_randkey)
    {
        retval = randkey_princ(princ.principal, FALSE, n_ks_tuple, ks_tuple);
        if (retval)
        {
            com_err("add_principal", retval, _("while randomizing key for \"%s\"."), canon);
            goto cleanup;
        }
        princ.attributes &= ~KRB5_KDB_DISALLOW_ALL_TIX;
        mask = KADM5_ATTRIBUTES;
        retval = kadm5_modify_principal(handle, &princ, mask);
        if (retval)
        {
            com_err("add_principal", retval, _("while clearing DISALLOW_ALL_TIX for \"%s\"."), canon);
            goto cleanup;
        }
    }
    printf("Principal \"%s\" created.\n", canon);
cleanup:
    krb5_free_principal(context, princ.principal);
    free(ks_tuple);
    free(canon);
    kadmin_free_tl_data(&princ.n_tl_data, &princ.tl_data);
}