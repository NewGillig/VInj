static gcry_err_code_t sexp_to_key(gcry_sexp_t sexp, int want_private, int use, const char *override_elems, gcry_mpi_t **retarray, gcry_module_t *retalgo, int *r_is_ecc)
{
    gcry_err_code_t err = 0;
    gcry_sexp_t list, l2;
    char *name;
    const char *elems;
    gcry_mpi_t *array;
    gcry_module_t module;
    gcry_pk_spec_t *pubkey;
    pk_extra_spec_t *extraspec;
    int is_ecc;
    list = gcry_sexp_find_token(sexp, want_private ? "private-key" : "public-key", 0);
    if (!list && !want_private)
        list = gcry_sexp_find_token(sexp, "private-key", 0);
    if (!list)
        return GPG_ERR_INV_OBJ;
    l2 = gcry_sexp_cadr(list);
    gcry_sexp_release(list);
    list = l2;
    name = _gcry_sexp_nth_string(list, 0);
    if (!name)
    {
        gcry_sexp_release(list);
        return GPG_ERR_INV_OBJ;
    }
    if (!strcmp(name, "ecc"))
        is_ecc = 2;
    else if (!strcmp(name, "ecdsa") || !strcmp(name, "ecdh"))
        is_ecc = 1;
    else
        is_ecc = 0;
    ath_mutex_lock(&pubkeys_registered_lock);
    if (is_ecc == 2 && (use & GCRY_PK_USAGE_SIGN))
        module = gcry_pk_lookup_name("ecdsa");
    else if (is_ecc == 2 && (use & GCRY_PK_USAGE_ENCR))
        module = gcry_pk_lookup_name("ecdh");
    else
        module = gcry_pk_lookup_name(name);
    ath_mutex_unlock(&pubkeys_registered_lock);
    gcry_free(name);
    if (!module)
    {
        gcry_sexp_release(list);
        return GPG_ERR_PUBKEY_ALGO;
    }
    else
    {
        pubkey = (gcry_pk_spec_t *)module->spec;
        extraspec = module->extraspec;
    }
    if (override_elems)
        elems = override_elems;
    else if (want_private)
        elems = pubkey->elements_skey;
    else
        elems = pubkey->elements_pkey;
    array = gcry_calloc(strlen(elems) + 1, sizeof(*array));
    if (!array)
        err = gpg_err_code_from_syserror();
    if (!err)
    {
        if (is_ecc)
            err = sexp_elements_extract_ecc(list, elems, array, extraspec, want_private);
        else
            err = sexp_elements_extract(list, elems, array, pubkey->name, 0);
    }
    gcry_sexp_release(list);
    if (err)
    {
        gcry_free(array);
        ath_mutex_lock(&pubkeys_registered_lock);
        _gcry_module_release(module);
        ath_mutex_unlock(&pubkeys_registered_lock);
    }
    else
    {
        *retarray = array;
        *retalgo = module;
        if (r_is_ecc)
            *r_is_ecc = is_ecc;
    }
    return err;
}