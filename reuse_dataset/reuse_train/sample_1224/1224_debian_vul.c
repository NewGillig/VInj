void Init_ossl_cipher(void)
{
    eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
    eCipherError = rb_define_class_under(cCipher, "CipherError", eOSSLError);
    rb_define_alloc_func(cCipher, ossl_cipher_alloc);
    rb_define_copy_func(cCipher, ossl_cipher_copy);
    rb_define_module_function(cCipher, "ciphers", ossl_s_ciphers, 0);
    rb_define_method(cCipher, "initialize", ossl_cipher_initialize, 1);
    rb_define_method(cCipher, "reset", ossl_cipher_reset, 0);
    rb_define_method(cCipher, "encrypt", ossl_cipher_encrypt, -1);
    rb_define_method(cCipher, "decrypt", ossl_cipher_decrypt, -1);
    rb_define_method(cCipher, "pkcs5_keyivgen", ossl_cipher_pkcs5_keyivgen, -1);
    rb_define_method(cCipher, "update", ossl_cipher_update, -1);
    rb_define_method(cCipher, "final", ossl_cipher_final, 0);
    rb_define_method(cCipher, "name", ossl_cipher_name, 0);
    rb_define_method(cCipher, "key=", ossl_cipher_set_key, 1);
    rb_define_method(cCipher, "auth_data=", ossl_cipher_set_auth_data, 1);
    rb_define_method(cCipher, "auth_tag=", ossl_cipher_set_auth_tag, 1);
    rb_define_method(cCipher, "auth_tag", ossl_cipher_get_auth_tag, -1);
    rb_define_method(cCipher, "auth_tag_len=", ossl_cipher_set_auth_tag_len, 1);
    rb_define_method(cCipher, "authenticated?", ossl_cipher_is_authenticated, 0);
    rb_define_method(cCipher, "key_len=", ossl_cipher_set_key_length, 1);
    rb_define_method(cCipher, "key_len", ossl_cipher_key_length, 0);
    rb_define_method(cCipher, "iv=", ossl_cipher_set_iv, 1);
    rb_define_method(cCipher, "iv_len=", ossl_cipher_set_iv_length, 1);
    rb_define_method(cCipher, "iv_len", ossl_cipher_iv_length, 0);
    rb_define_method(cCipher, "block_size", ossl_cipher_block_size, 0);
    rb_define_method(cCipher, "padding=", ossl_cipher_set_padding, 1);
    id_auth_tag_len = rb_intern_const("auth_tag_len");
}