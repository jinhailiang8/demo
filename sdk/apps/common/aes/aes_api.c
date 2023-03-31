/* This is a small and portable implementation of the AES */

#define CBC 0
#define CTR 0
#define ECB 1

#include "system/includes.h"
#include "aes/aes.h"

/* --------------------------------------------------------------------------*/
/**
 * @brief ECB mode, software encrypt, 128bit
 *
 * @param key[16] 128bit
 * @param plaintext[16] (input) 16byte
 * @param encrypt[16] (output) 16byte
 *
 * @return 0: succ, others: err
 */
/* ----------------------------------------------------------------------------*/
int aes128_ecb_encrypt_software(unsigned char key[16], unsigned char plaintext[16], unsigned char encrypt[16])
{
    struct AES_ctx ctx;
    AES_init_ctx(&ctx, key);
    memcpy(encrypt, plaintext, 16);
    AES_ECB_encrypt(&ctx, encrypt);

    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief ECB mode, software decrypt, 128bit
 *
 * @param key[16] 128bit
 * @param encrypt[16] (input) 16byte
 * @param plaintext[16] (output) 16byte
 *
 * @return 0: succ, others: err
 */
/* ----------------------------------------------------------------------------*/
int aes128_ecb_decrypt_software(unsigned char key[16], unsigned char encrypt[16], unsigned char plaintext[16])
{
    struct AES_ctx ctx;
    AES_init_ctx(&ctx, key);
    memcpy(plaintext, encrypt, 16);
    AES_ECB_decrypt(&ctx, plaintext);
    return 0;
}


/* --------------------------------------------------------------------------*/
/**
 * @brief aes test demo
 */
/* ----------------------------------------------------------------------------*/
void aes_software_test(void)
{
    uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    uint8_t plt[] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a };
    uint8_t enc[] = { 0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60, 0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97 };

    /* uint8_t key[] = { 0xbf, 0x01, 0xfb, 0x9d, 0x4e, 0xf3, 0xbc, 0x36, 0xd8, 0x74, 0xf5, 0x39, 0x41, 0x38, 0x68, 0x4c }; */
    /* uint8_t plt[] = { 0x13, 0x02, 0xf1, 0xe0, 0xdf, 0xce, 0xbd, 0xac, 0x79, 0x68, 0x57, 0x46, 0x35, 0x24, 0x13, 0x02 }; */
    /* uint8_t enc[] = { 0x66, 0xc6, 0xc2, 0x27, 0x8e, 0x3b, 0x8e, 0x05, 0x3e, 0x7e, 0xa3, 0x26, 0x52, 0x1b, 0xad, 0x99 }; */

    /* uint8_t key[] = {0x4C, 0x68, 0x38, 0x41, 0x39, 0xF5, 0x74, 0xD8, 0x36, 0xBC, 0xF3, 0x4E, 0x9D, 0xFB, 0x01, 0xBF }; */
    /* uint8_t plt[] = {0x02, 0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79, 0xac, 0xbd, 0xce, 0xdf, 0xe0, 0xf1, 0x02, 0x13 }; */
    /* uint8_t enc[] = {0x99, 0xad, 0x1b, 0x52, 0x26, 0xa3, 0x7e, 0x3e, 0x05, 0x8e, 0x3b, 0x8e, 0x27, 0xc2, 0xc6, 0x66 }; */

    uint8_t out[16];

    /* extern int aes128_start_enc(unsigned char key[16], unsigned char plaintext[16], unsigned char encrypt[16]); */
    /* aes128_start_enc(key, plt, out); */

    printf("aes128_ecb_encrypt_software : \n");
    aes128_ecb_encrypt_software(key, plt, out);
    put_buf(out, 16);
    if (0 == memcmp((char *) out, (char *) enc, 16)) {
        printf("SUCCESS!\n");
    } else {
        printf("FAILURE!\n");
    }

    printf("aes128_ecb_decrypt_software : \n");
    aes128_ecb_decrypt_software(key, enc, out);
    put_buf(out, 16);
    if (0 == memcmp((char *) out, (char *) plt, 16)) {
        printf("SUCCESS!\n");
    } else {
        printf("FAILURE!\n");
    }

    while (1);
}

