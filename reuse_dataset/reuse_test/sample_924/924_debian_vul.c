static const char *cmd_hash_engine(cmd_parms *cmd, void *_dcfg, const char *p1)
{
    directory_config *dcfg = (directory_config *)_dcfg;
    if (dcfg == NULL)
        return NULL;
    if (strcasecmp(p1, "on") == 0)
    {
        dcfg->hash_is_enabled = HASH_ENABLED;
        dcfg->hash_enforcement = HASH_ENABLED;
    }
    else if (strcasecmp(p1, "off") == 0)
    {
        dcfg->hash_is_enabled = HASH_DISABLED;
        dcfg->hash_enforcement = HASH_DISABLED;
    }
    else
        return apr_psprintf(cmd->pool, "ModSecurity: Invalid value for SecRuleEngine: %s", p1);
    return NULL;
}