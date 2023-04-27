static VALUE ossl_asn1_decode0(unsigned char **pp, long length, long *offset, int depth, int yield, long *num_read)
{
    unsigned char *start, *p;
    const unsigned char *p0;
    long len = 0, inner_read = 0, off = *offset, hlen;
    int tag, tc, j;
    VALUE asn1data, tag_class;
    p = *pp;
    start = p;
    p0 = p;
    j = ASN1_get_object(&p0, &len, &tag, &tc, length);
    p = (unsigned char *)p0;
    if (j & 0x80)
        ossl_raise(eASN1Error, NULL);
    if (len > length)
        ossl_raise(eASN1Error, "value is too short");
    if ((tc & V_ASN1_PRIVATE) == V_ASN1_PRIVATE)
        tag_class = sym_PRIVATE;
    else if ((tc & V_ASN1_CONTEXT_SPECIFIC) == V_ASN1_CONTEXT_SPECIFIC)
        tag_class = sym_CONTEXT_SPECIFIC;
    else if ((tc & V_ASN1_APPLICATION) == V_ASN1_APPLICATION)
        tag_class = sym_APPLICATION;
    else
        tag_class = sym_UNIVERSAL;
    hlen = p - start;
    if (yield)
    {
        VALUE arg = rb_ary_new();
        rb_ary_push(arg, LONG2NUM(depth));
        rb_ary_push(arg, LONG2NUM(*offset));
        rb_ary_push(arg, LONG2NUM(hlen));
        rb_ary_push(arg, LONG2NUM(len));
        rb_ary_push(arg, (j & V_ASN1_CONSTRUCTED) ? Qtrue : Qfalse);
        rb_ary_push(arg, ossl_asn1_class2sym(tc));
        rb_ary_push(arg, INT2NUM(tag));
        rb_yield(arg);
    }
    if (j & V_ASN1_CONSTRUCTED)
    {
        *pp += hlen;
        off += hlen;
        asn1data = int_ossl_asn1_decode0_cons(pp, length, len, &off, depth, yield, j, tag, tag_class, &inner_read);
        inner_read += hlen;
    }
    else
    {
        if ((j & 0x01) && (len == 0))
            ossl_raise(eASN1Error, "Infinite length for primitive value");
        asn1data = int_ossl_asn1_decode0_prim(pp, len, hlen, tag, tag_class, &inner_read);
        off += hlen + len;
    }
    if (num_read)
        *num_read = inner_read;
    if (len != 0 && inner_read != hlen + len)
    {
        ossl_raise(eASN1Error, "Type mismatch. Bytes read: %ld Bytes available: %ld", inner_read, hlen + len);
    }
    *offset = off;
    return asn1data;
}