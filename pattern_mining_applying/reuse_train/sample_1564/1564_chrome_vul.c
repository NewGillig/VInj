static int32_t _appendKeywordsToLanguageTag(const char *localeID, char *appendAt, int32_t capacity, UBool strict, UBool hadPosix, UErrorCode *status)
{
    char buf[ULOC_KEYWORD_AND_VALUES_CAPACITY];
    char attrBuf[ULOC_KEYWORD_AND_VALUES_CAPACITY] = {0};
    int32_t attrBufLength = 0;
    UEnumeration *keywordEnum = NULL;
    int32_t reslen = 0;
    keywordEnum = uloc_openKeywords(localeID, status);
    if (U_FAILURE(*status) && !hadPosix)
    {
        uenum_close(keywordEnum);
        return 0;
    }
    if (keywordEnum != NULL || hadPosix)
    {
        int32_t len;
        const char *key;
        ExtensionListEntry *firstExt = NULL;
        ExtensionListEntry *ext;
        AttributeListEntry *firstAttr = NULL;
        AttributeListEntry *attr;
        char *attrValue;
        char extBuf[ULOC_KEYWORD_AND_VALUES_CAPACITY];
        char *pExtBuf = extBuf;
        int32_t extBufCapacity = sizeof(extBuf);
        const char *bcpKey, *bcpValue;
        UErrorCode tmpStatus = U_ZERO_ERROR;
        int32_t keylen;
        UBool isBcpUExt;
        while (TRUE)
        {
            key = uenum_next(keywordEnum, NULL, status);
            if (key == NULL)
            {
                break;
            }
            len = uloc_getKeywordValue(localeID, key, buf, sizeof(buf), &tmpStatus);
            if (U_FAILURE(tmpStatus) || tmpStatus == U_STRING_NOT_TERMINATED_WARNING)
            {
                if (strict)
                {
                    *status = U_ILLEGAL_ARGUMENT_ERROR;
                    break;
                }
                tmpStatus = U_ZERO_ERROR;
                continue;
            }
            keylen = (int32_t)uprv_strlen(key);
            isBcpUExt = (keylen > 1);
            if (uprv_strcmp(key, LOCALE_ATTRIBUTE_KEY) == 0)
            {
                if (len > 0)
                {
                    int32_t i = 0;
                    while (TRUE)
                    {
                        attrBufLength = 0;
                        for (; i < len; i++)
                        {
                            if (buf[i] != '-')
                            {
                                attrBuf[attrBufLength++] = buf[i];
                            }
                            else
                            {
                                i++;
                                break;
                            }
                        }
                        if (attrBufLength > 0)
                        {
                            attrBuf[attrBufLength] = 0;
                        }
                        else if (i >= len)
                        {
                            break;
                        }
                        attr = (AttributeListEntry *)uprv_malloc(sizeof(AttributeListEntry));
                        if (attr == NULL)
                        {
                            *status = U_MEMORY_ALLOCATION_ERROR;
                            break;
                        }
                        attrValue = (char *)uprv_malloc(attrBufLength + 1);
                        if (attrValue == NULL)
                        {
                            *status = U_MEMORY_ALLOCATION_ERROR;
                            break;
                        }
                        uprv_strcpy(attrValue, attrBuf);
                        attr->attribute = attrValue;
                        if (!_addAttributeToList(&firstAttr, attr))
                        {
                            uprv_free(attr);
                            uprv_free(attrValue);
                            if (strict)
                            {
                                *status = U_ILLEGAL_ARGUMENT_ERROR;
                                break;
                            }
                        }
                    }
                    bcpKey = LOCALE_ATTRIBUTE_KEY;
                    bcpValue = NULL;
                }
            }
            else if (isBcpUExt)
            {
                bcpKey = uloc_toUnicodeLocaleKey(key);
                if (bcpKey == NULL)
                {
                    if (strict)
                    {
                        *status = U_ILLEGAL_ARGUMENT_ERROR;
                        break;
                    }
                    continue;
                }
                bcpValue = uloc_toUnicodeLocaleType(key, buf);
                if (bcpValue == NULL)
                {
                    if (strict)
                    {
                        *status = U_ILLEGAL_ARGUMENT_ERROR;
                        break;
                    }
                    continue;
                }
                if (bcpValue == buf)
                {
                    int32_t bcpValueLen = uprv_strlen(bcpValue);
                    if (bcpValueLen < extBufCapacity)
                    {
                        uprv_strcpy(pExtBuf, bcpValue);
                        T_CString_toLowerCase(pExtBuf);
                        bcpValue = pExtBuf;
                        pExtBuf += (bcpValueLen + 1);
                        extBufCapacity -= (bcpValueLen + 1);
                    }
                    else
                    {
                        if (strict)
                        {
                            *status = U_ILLEGAL_ARGUMENT_ERROR;
                            break;
                        }
                        continue;
                    }
                }
            }
            else
            {
                if (*key == PRIVATEUSE)
                {
                    if (!_isPrivateuseValueSubtags(buf, len))
                    {
                        if (strict)
                        {
                            *status = U_ILLEGAL_ARGUMENT_ERROR;
                            break;
                        }
                        continue;
                    }
                }
                else
                {
                    if (!_isExtensionSingleton(key, keylen) || !_isExtensionSubtags(buf, len))
                    {
                        if (strict)
                        {
                            *status = U_ILLEGAL_ARGUMENT_ERROR;
                            break;
                        }
                        continue;
                    }
                }
                bcpKey = key;
                if ((len + 1) < extBufCapacity)
                {
                    uprv_memcpy(pExtBuf, buf, len);
                    bcpValue = pExtBuf;
                    pExtBuf += len;
                    *pExtBuf = 0;
                    pExtBuf++;
                    extBufCapacity -= (len + 1);
                }
                else
                {
                    *status = U_ILLEGAL_ARGUMENT_ERROR;
                    break;
                }
            }
            ext = (ExtensionListEntry *)uprv_malloc(sizeof(ExtensionListEntry));
            if (ext == NULL)
            {
                *status = U_MEMORY_ALLOCATION_ERROR;
                break;
            }
            ext->key = bcpKey;
            ext->value = bcpValue;
            if (!_addExtensionToList(&firstExt, ext, TRUE))
            {
                uprv_free(ext);
                if (strict)
                {
                    *status = U_ILLEGAL_ARGUMENT_ERROR;
                    break;
                }
            }
        }
        if (hadPosix)
        {
            ext = (ExtensionListEntry *)uprv_malloc(sizeof(ExtensionListEntry));
            if (ext == NULL)
            {
                *status = U_MEMORY_ALLOCATION_ERROR;
                goto cleanup;
            }
            ext->key = POSIX_KEY;
            ext->value = POSIX_VALUE;
            if (!_addExtensionToList(&firstExt, ext, TRUE))
            {
                uprv_free(ext);
            }
        }
        if (U_SUCCESS(*status) && (firstExt != NULL || firstAttr != NULL))
        {
            UBool startLDMLExtension = FALSE;
            for (ext = firstExt; ext; ext = ext->next)
            {
                if (!startLDMLExtension && uprv_strlen(ext->key) > 1)
                {
                    if (reslen < capacity)
                    {
                        *(appendAt + reslen) = SEP;
                    }
                    reslen++;
                    if (reslen < capacity)
                    {
                        *(appendAt + reslen) = LDMLEXT;
                    }
                    reslen++;
                    startLDMLExtension = TRUE;
                }
                if (uprv_strcmp(ext->key, LOCALE_ATTRIBUTE_KEY) == 0)
                {
                    for (attr = firstAttr; attr; attr = attr->next)
                    {
                        if (reslen < capacity)
                        {
                            *(appendAt + reslen) = SEP;
                        }
                        reslen++;
                        len = (int32_t)uprv_strlen(attr->attribute);
                        if (reslen < capacity)
                        {
                            uprv_memcpy(appendAt + reslen, attr->attribute, uprv_min(len, capacity - reslen));
                        }
                        reslen += len;
                    }
                }
                else
                {
                    if (reslen < capacity)
                    {
                        *(appendAt + reslen) = SEP;
                    }
                    reslen++;
                    len = (int32_t)uprv_strlen(ext->key);
                    if (reslen < capacity)
                    {
                        uprv_memcpy(appendAt + reslen, ext->key, uprv_min(len, capacity - reslen));
                    }
                    reslen += len;
                    if (reslen < capacity)
                    {
                        *(appendAt + reslen) = SEP;
                    }
                    reslen++;
                    len = (int32_t)uprv_strlen(ext->value);
                    if (reslen < capacity)
                    {
                        uprv_memcpy(appendAt + reslen, ext->value, uprv_min(len, capacity - reslen));
                    }
                    reslen += len;
                }
            }
        }
    cleanup:
        ext = firstExt;
        while (ext != NULL)
        {
            ExtensionListEntry *tmpExt = ext->next;
            uprv_free(ext);
            ext = tmpExt;
        }
        attr = firstAttr;
        while (attr != NULL)
        {
            AttributeListEntry *tmpAttr = attr->next;
            char *pValue = (char *)attr->attribute;
            uprv_free(pValue);
            uprv_free(attr);
            attr = tmpAttr;
        }
        uenum_close(keywordEnum);
        if (U_FAILURE(*status))
        {
            return 0;
        }
    }
    return u_terminateChars(appendAt, capacity, reslen, status);
}