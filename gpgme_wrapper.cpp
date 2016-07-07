#include "gpgme_wrapper.h"

bool gpgme_wrapper::doInit() {
//do initialization code here
//check for already loaded
    dl.Load(dl.CanonicalizeName("gpgme"));
    if (!dl.IsLoaded())
    return 0;

    //Loads all the functions into our function pointers
    gpgme_new_ptr =                 (gpgme_error_t (*) (gpgme_ctx_t *ctx))dl.GetSymbol(wxT("gpgme_new"));
    gpgme_strerror_ptr =            (const char * (*) (gpgme_error_t err))dl.GetSymbol(wxT("gpgme_strerror"));
    gpgme_check_version_ptr =       (const char * (*) (const char *required_version))dl.GetSymbol(wxT("gpgme_check_version"));
    gpgme_op_keylist_start_ptr =    (gpgme_error_t (*) (gpgme_ctx_t ctx, const char *pattern, int secret_only))dl.GetSymbol(wxT("gpgme_op_keylist_start"));
    gpgme_op_keylist_next_ptr =     (gpgme_error_t (*) (gpgme_ctx_t ctx, gpgme_key_t *r_key))dl.GetSymbol(wxT("gpgme_op_keylist_next"));
    gpgme_op_keylist_end_ptr =      (gpgme_error_t (*) (gpgme_ctx_t ctx))dl.GetSymbol(wxT("gpgme_op_keylist_end"));
    gpgme_get_key_ptr =             (gpgme_error_t (*) (gpgme_ctx_t ctx, const char *fpr, gpgme_key_t *r_key, int secret))dl.GetSymbol(wxT("gpgme_get_key"));
    gpgme_key_get_string_attr_ptr = (const char * (*) (gpgme_key_t key, gpgme_attr_t what, const void *reserved, int idx))dl.GetSymbol(wxT("gpgme_key_get_string_attr"));
    gpgme_data_new_from_mem_ptr =   (gpgme_error_t (*) (gpgme_data_t *dh, const char *buffer, size_t size, int copy))dl.GetSymbol(wxT("gpgme_data_new_from_mem"));
    gpgme_data_new_ptr =            (gpgme_error_t (*) (gpgme_data_t *dh))dl.GetSymbol(wxT("gpgme_data_new"));
    gpgme_data_release_and_get_mem_ptr = (char * (*) (gpgme_data_t dh, size_t *length))dl.GetSymbol(wxT("gpgme_data_release_and_get_mem"));
    gpgme_signers_clear_ptr =       (void (*) (gpgme_ctx_t ctx))dl.GetSymbol(wxT("gpgme_signers_clear"));
    gpgme_signers_add_ptr =         (gpgme_error_t (*) (gpgme_ctx_t ctx, const gpgme_key_t key))dl.GetSymbol(wxT("gpgme_signers_add"));
    gpgme_op_encrypt_ptr =          (gpgme_error_t (*) (gpgme_ctx_t ctx, gpgme_key_t recp[], gpgme_encrypt_flags_t flags, gpgme_data_t plain, gpgme_data_t cipher))dl.GetSymbol(wxT("gpgme_op_encrypt"));
    gpgme_op_encrypt_sign_ptr =     (gpgme_error_t (*) (gpgme_ctx_t ctx, gpgme_key_t recp[], gpgme_encrypt_flags_t flags, gpgme_data_t plain, gpgme_data_t cipher))dl.GetSymbol(wxT("gpgme_op_encrypt_sign"));
    gpgme_set_protocol_ptr =        (gpgme_error_t (*) (gpgme_ctx_t ctx, gpgme_protocol_t proto))dl.GetSymbol(wxT("gpgme_set_protocol"));
    gpgme_set_armor_ptr =           (void (*) (gpgme_ctx_t ctx, int yes))dl.GetSymbol(wxT("gpgme_set_armor"));

    //Starts the actual init
    gpgme_check_version_ptr(nullptr);

    gpgerr = gpgme_new_ptr(&gpgcon);
    if (gpgerr != GPG_ERR_NO_ERROR) {
        wxMessageBox(_("GPG returned the error: ") + _(gpgme_strerror_ptr(gpgerr)));
        return 0;
    }



    enabled = true;
    return 1;



    //set flag true on success
}

void gpgme_wrapper::getAllKeys(wxArrayString * keys) {
    gpgme_key_t tmpKey;
    keys->Empty();
    gpgerr = gpgme_op_keylist_start_ptr(gpgcon, 0, 0);
    if (gpgerr != GPG_ERR_NO_ERROR)
    return;
    while (gpgme_op_keylist_next_ptr(gpgcon, &tmpKey) != GPG_ERR_EOF) {
    //gpgme_op_keylist_next_ptr(gpgcon, &tmpKey);
        if (tmpKey == 0)
            break;
        keys->Insert(_(gpgme_key_get_string_attr_ptr(tmpKey, GPGME_ATTR_KEYID, 0, 0)).Right(8), 0);
    }
    gpgme_op_keylist_end_ptr(gpgcon);

}
bool gpgme_wrapper::encryptAndSign(wxString encryptKey, wxString sigKey, char * plaintext, char * cipher) {
    gpgme_data_t plain_data;
    gpgme_data_t cipher_data;
    gpgme_key_t         key[2]      = { NULL, NULL };
    gpgme_key_t sig_key;
    size_t * buf_len;
    char *buf;
    char key_buf[64] = {0};
    buf_len = new size_t;
    char *ndx;

    gpgerr = gpgme_data_new_from_mem_ptr(&plain_data, plaintext, strlen(plaintext), 1);
    if (gpgerr != GPG_ERR_NO_ERROR) {
        wxMessageBox(_("GPG returned the error: ") + _(gpgme_strerror_ptr(gpgerr)));
        return 0;
    }
    gpgerr = gpgme_data_new_ptr(&cipher_data);
    if (gpgerr != GPG_ERR_NO_ERROR) {
        wxMessageBox(_("GPG returned the error: ") + _(gpgme_strerror_ptr(gpgerr)));
        return 0;
    }
    strcpy(key_buf, (const char*)encryptKey.mb_str(wxConvUTF8));
    gpgerr = gpgme_get_key_ptr(gpgcon, key_buf, &key[0], 0);
    if (gpgerr != GPG_ERR_NO_ERROR) {
        wxMessageBox(_("GPG returned the error: ") + _(gpgme_strerror_ptr(gpgerr)));
        return 0;
    }
    gpgme_set_protocol_ptr(gpgcon, GPGME_PROTOCOL_OpenPGP);
    gpgme_set_armor_ptr(gpgcon, 0);

    gpgme_signers_clear_ptr(gpgcon);
    if (sigKey.CmpNoCase(wxT("None")) == 0) {
        gpgerr = gpgme_op_encrypt_ptr(gpgcon, key, GPGME_ENCRYPT_ALWAYS_TRUST, plain_data, cipher_data);
        if (gpgerr != GPG_ERR_NO_ERROR) {
            wxMessageBox(_("GPG returned the error: ") + _(gpgme_strerror_ptr(gpgerr)));
            return 0;
        }



    } else {
        strcpy(key_buf, (const char*)sigKey.mb_str(wxConvUTF8));
        gpgerr = gpgme_get_key_ptr(gpgcon, key_buf, &sig_key, 0);
        if (gpgerr != GPG_ERR_NO_ERROR) {
            wxMessageBox(_("GPG returned the error: ") + _(gpgme_strerror_ptr(gpgerr)));
            return 0;
        }
        gpgme_signers_add_ptr(gpgcon, sig_key);
        gpgme_op_encrypt_sign_ptr(gpgcon, key, GPGME_ENCRYPT_ALWAYS_TRUST, plain_data, cipher_data);
    }
    buf = gpgme_data_release_and_get_mem_ptr(cipher_data, buf_len);
    if (buf == nullptr)
        return 0;
    wxBase64Encode(cipher, 4096, buf, *buf_len);
    if((ndx = strchr(cipher, '=')) != NULL)
        *ndx = '\0';
//base64 encode before returning
return 1;
}