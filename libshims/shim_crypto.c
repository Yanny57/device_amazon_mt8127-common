/*
 * BoringSSL in R dropped RSA_verify_PKCS1_PSS() and EVP_EncryptFinal(); the
 * Nougat Widevine engine (libwvdrmengine) still imports both.
 * RSA_verify_PKCS1_PSS is RSA_verify_PKCS1_PSS_mgf1() with the MGF1 hash
 * defaulting to the message hash (NULL); EVP_EncryptFinal is EVP_EncryptFinal_ex.
 * Both are resolved at runtime from the libcrypto the engine already loaded, so
 * the shim does not pull libcrypto into every process that uses libshim_liblog.
 */
#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>

static void* crypto_sym(const char* name) {
    void* lib = dlopen("libcrypto.so", RTLD_NOW | RTLD_NOLOAD);
    if (lib == NULL) lib = dlopen("libcrypto.so", RTLD_NOW);
    return lib != NULL ? dlsym(lib, name) : NULL;
}

typedef int (*pss_mgf1_fn)(void* rsa, const uint8_t* mHash, const void* Hash,
                           const void* mgf1Hash, const uint8_t* EM, int sLen);

int RSA_verify_PKCS1_PSS(void* rsa, const uint8_t* mHash, const void* Hash, const uint8_t* EM,
                         int sLen) {
    static pss_mgf1_fn fn;
    if (fn == NULL) fn = (pss_mgf1_fn)crypto_sym("RSA_verify_PKCS1_PSS_mgf1");
    if (fn == NULL) return 0;
    return fn(rsa, mHash, Hash, NULL, EM, sLen);
}

typedef int (*encrypt_final_fn)(void* ctx, uint8_t* out, int* out_len);

int EVP_EncryptFinal(void* ctx, uint8_t* out, int* out_len) {
    static encrypt_final_fn fn;
    if (fn == NULL) fn = (encrypt_final_fn)crypto_sym("EVP_EncryptFinal_ex");
    if (fn == NULL) return 0;
    return fn(ctx, out, out_len);
}
