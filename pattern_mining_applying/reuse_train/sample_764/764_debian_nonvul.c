void kadmin_modprinc(int argc, char *argv[])
{
    kadm5_principal_ent_rec princ, oldprinc;
    krb5_principal kprinc = NULL;
    long mask;
    krb5_error_code retval;
    char *pass, *canon = NULL;
    krb5_boolean randkey = FALSE;
    int n_ks_tuple = 0;
    krb5_key_salt_tuple *ks_tuple = NULL;
    if (argc < 2)
    {
        kadmin_modprinc_usage();
        return;
    }
    memset(&oldprinc, 0, sizeof(oldprinc));
    memset(&princ, 0, sizeof(princ));
    retval = kadmin_parse_name(argv[argc - 1], &kprinc);
    if (retval)
    {
        com_err("modify_principal", retval, _("while parsing principal"));
        return;
    }
    retval = krb5_unparse_name(context, kprinc, &canon);
    if (retval)
    {
        com_err("modify_principal", retval, _("while canonicalizing principal"));
        goto cleanup;
    }
    retval = kadm5_get_principal(handle, kprinc, &oldprinc, KADM5_PRINCIPAL_NORMAL_MASK);
    if (retval)
    {
        com_err("modify_principal", retval, _("while getting \"%s\"."), canon);
        goto cleanup;
    }
    princ.attributes = oldprinc.attributes;
    kadm5_free_principal_ent(handle, &oldprinc);
    retval = kadmin_parse_princ_args(argc, argv, &princ, &mask, &pass, &randkey, &ks_tuple, &n_ks_tuple, "modify_principal");
    if (retval || ks_tuple != NULL || randkey || pass)
    {
        kadmin_modprinc_usage();
        goto cleanup;
    }
    if (mask & KADM5_POLICY)
    {
        if (!policy_exists(princ.policy))
        {
            fprintf(stderr, _("WARNING: policy \"%s\" does not exist\n"), princ.policy);
        }
    }
    if (mask)
    {
        retval = kadm5_modify_principal(handle, &princ, mask);
    }
    if (retval)
    {
        com_err("modify_principal", retval, _("while modifying \"%s\"."), canon);
        goto cleanup;
    }
    printf(_("Principal \"%s\" modified.\n"), canon);
cleanup:
    krb5_free_principal(context, kprinc);
    krb5_free_principal(context, princ.principal);
    kadmin_free_tl_data(&princ.n_tl_data, &princ.tl_data);
    free(canon);
    free(ks_tuple);
}