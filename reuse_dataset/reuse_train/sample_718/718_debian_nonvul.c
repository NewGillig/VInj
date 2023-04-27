void DefaultTTFEnglishNames(struct ttflangname *dummy, SplineFont *sf)
{
    time_t now;
    struct tm *tm;
    char buffer[200];
    if (dummy->names[ttf_copyright] == NULL || *dummy->names[ttf_copyright] == '\0')
        dummy->names[ttf_copyright] = utf8_verify_copy(sf->copyright);
    if (dummy->names[ttf_family] == NULL || *dummy->names[ttf_family] == '\0')
        dummy->names[ttf_family] = utf8_verify_copy(sf->familyname);
    if (dummy->names[ttf_subfamily] == NULL || *dummy->names[ttf_subfamily] == '\0')
        dummy->names[ttf_subfamily] = utf8_verify_copy(SFGetModifiers(sf));
    if (dummy->names[ttf_uniqueid] == NULL || *dummy->names[ttf_uniqueid] == '\0')
    {
        time(&now);
        tm = localtime(&now);
        snprintf(buffer, sizeof(buffer), "%s : %s : %d-%d-%d", BDFFoundry ? BDFFoundry : TTFFoundry ? TTFFoundry
                                                                                                    : "FontForge 2.0",
                 sf->fullname != NULL ? sf->fullname : sf->fontname, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
        dummy->names[ttf_uniqueid] = copy(buffer);
    }
    if (dummy->names[ttf_fullname] == NULL || *dummy->names[ttf_fullname] == '\0')
        dummy->names[ttf_fullname] = utf8_verify_copy(sf->fullname);
    if (dummy->names[ttf_version] == NULL || *dummy->names[ttf_version] == '\0')
    {
        if (sf->subfontcnt != 0)
            sprintf(buffer, "Version %f ", (double)sf->cidversion);
        else if (sf->version != NULL)
            sprintf(buffer, "Version %.20s ", sf->version);
        else
            strcpy(buffer, "Version 1.0");
        dummy->names[ttf_version] = copy(buffer);
    }
    if (dummy->names[ttf_postscriptname] == NULL || *dummy->names[ttf_postscriptname] == '\0')
        dummy->names[ttf_postscriptname] = utf8_verify_copy(sf->fontname);
}