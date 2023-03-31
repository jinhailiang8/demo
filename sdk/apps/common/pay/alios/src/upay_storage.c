#include "system/includes.h"
#include "fs.h"
#include "sdfile.h"
#include "asm/crc16.h"
#include "upay_common.h"


#if 0
#pragma bss_seg(  ".upay_bss")
#pragma data_seg(  ".upay_data")
#pragma const_seg(  ".upay_const")
#pragma code_seg(  ".upay_code")
#endif

static OS_MUTEX upay_mutex;
#define upay_mutex_init() 	os_mutex_create(&upay_mutex)
#define upay_mutex_pend() 	os_mutex_pend(&upay_mutex, portMAX_DELAY)
#define upay_mutex_post() 	os_mutex_post(&upay_mutex)

typedef struct UPAY_INFO {
    u16 head_crc;   //数据头CRC
    u16 data_crc;	//所有数据总长度CRC
    u16 item_id;    //数据ID号
    u16 len;        //数据长度，不包括头

    u8  aes_fix_flag;   //是否需要aes加解密补齐
    u8	end_flag;   //1:表示读完此次此ID内容标志
    u16 rlen;       //表示已经读出的数据长度，分批读出数据需要16byte对齐
    u8	resv[4];    //预留信息

    u8  aes_fix[16];    //aes加解密补齐buf
} _UPAY_INFO;

static int part_addr = 0;
static int part_len = 0;
/* static u16 item_buf[] = {512, 512, 512, 8, 128, 128, 1}; */
#define ITEM_MAX		7
static u16 item_len[ITEM_MAX] = {0};

static char test_key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
int aes128_ecb_encrypt_software(unsigned char key[16], unsigned char plaintext[16], unsigned char encrypt[16]);
int aes128_ecb_decrypt_software(unsigned char key[16], unsigned char encrypt[16], unsigned char plaintext[16]);
int norflash_write(struct device *device, void *buf, u32 len, u32 offset);
int norflash_erase(u32 cmd, u32 addr);
int norflash_origin_read(u8 *buf, u32 offset, u32 len);

static u16 upay_get_first_one(u16 n)
{
    u16 pos = 0;
    for (pos = 0; pos < 16; pos++) {
        if (n & BIT(pos)) {
            return pos;
        }
    }
    return 0xff;
}

int upay_search_next_id(int next_addr)
{
    if (next_addr < part_addr || next_addr >  part_addr + part_len) {
        return  -1;
    }
    _UPAY_INFO find_head = {0};
    int i = next_addr;
    for (; i < part_addr + part_len;) {
        norflash_origin_read((u8 *)&find_head, i, sizeof(_UPAY_INFO));
        if (find_head.head_crc == CRC16((u8 *)&find_head + 2, sizeof(_UPAY_INFO) - 2)) {
            return i;
        }
        i += sizeof(_UPAY_INFO);
    }
    return part_addr + part_len;
}

int upay_search_id_info(_UPAY_INFO *info, u16 item_id, u8 read_flag)
{
    _UPAY_INFO find_head = {0};
    int i = part_addr;
    for (; i < part_addr + part_len;) {
        norflash_origin_read((u8 *)&find_head, i, sizeof(_UPAY_INFO));
        /* printf(">>>[test]:i = %d\n", i); */
        /* put_buf(&find_head, 32); */
        if (find_head.head_crc != CRC16((u8 *)&find_head + 2, sizeof(_UPAY_INFO) - 2) || (0 == find_head.head_crc)) {
            if (read_flag) {
                i += sizeof(_UPAY_INFO);
                continue;
            }
            return i;
        }
        if (item_id == find_head.item_id) {
            memcpy((u8 *)info, (u8 *)&find_head, sizeof(_UPAY_INFO));
            /* return i + sizeof(_UPAY_INFO); */
            return i;
        }
        i += sizeof(_UPAY_INFO);
        i += find_head.len;
        i = (i + sizeof(_UPAY_INFO) - 1) / sizeof(_UPAY_INFO) * sizeof(_UPAY_INFO);
    }
    return -1;
}

#if 0
int upay_enc_dec_bank(char *buf, int len, int dec_flag)
{
    unsigned char out[16];
    int tmp_len = len;
    len = (len + 15) / 16 * 16;
    char *tmp_buf = (char *)zalloc(len);  //buf长度可能不是16的整数
    ASSERT(tmp_buf);
    memcpy(tmp_buf, buf, tmp_len);
    for (int i = 0; i < len; i += 16) {
        if (dec_flag) {
            aes128_ecb_decrypt_software((unsigned char *)test_key, (unsigned char *)tmp_buf + i, (unsigned char *)out);
        } else {
            aes128_ecb_encrypt_software((unsigned char *)test_key, (unsigned char *)tmp_buf + i, (unsigned char *)out);
        }
        memcpy(tmp_buf + i, out, sizeof(out));
    }
    memcpy(buf, tmp_buf, tmp_len);
    if (tmp_buf) {
        free(tmp_buf);
    }
    return len;
}
#endif

int upay_enc_dec_bank(_UPAY_INFO *info, char *buf, int len, int dec_flag)
{
    unsigned char out[16];
    unsigned char in[16];
    int tmp_len = 0;
    /* len = (len + 15) / 16 * 16; */
    for (int i = 0; i < len; i += 16) {
        tmp_len = 16;
        memset(in, 0, 16);
        if (dec_flag) {
            if (i + 16 > len) {
                tmp_len = len - i;
                if (info->aes_fix_flag) {
                    memcpy(in, info->aes_fix, 16);
                }
            } else {
                memcpy(in, buf + i, tmp_len);
            }
            aes128_ecb_decrypt_software((unsigned char *)test_key, (unsigned char *)in, (unsigned char *)out);
        } else {
            if (i + 16 > len) {
                tmp_len = len - i;
            }
            memcpy(in, buf + i, tmp_len);
            aes128_ecb_encrypt_software((unsigned char *)test_key, (unsigned char *)in, (unsigned char *)out);
            if (tmp_len != 16) {
                memcpy(info->aes_fix, out, 16);
                info->aes_fix_flag = 1;
            }
        }
        memcpy(buf + i, out, tmp_len);
    }
    return len;
}

extern u8 *get_norflash_uuid(void);
int upay_init(const char *usr_name)
{
    static FILE *pfile = NULL;
    if (!usr_name) {
        return -1;
    }
    upay_mutex_init();
    char path[64] = "mnt/sdfile/EXT_RESERVED/";
    memcpy(path + strlen(path), usr_name, strlen(usr_name));
    y_printf(">>>[test]:goto open path : %s\n", path);
    pfile = fopen(path, "rw");
    if (!pfile) {
        return -1;
    }
    struct vfs_attr attr = {0};
    fget_attrs(pfile, &attr);
    part_addr = attr.sclust;
    part_len = attr.fsize;
    u32 sdfile_cpu_addr2flash_addr(u32 offset);
    part_addr = sdfile_cpu_addr2flash_addr(part_addr);

    printf("\n\n\n upay init key : \n\n\n");
    put_buf(test_key, 16);

    u32 flash_uuid[4];
    extern u8 *get_norflash_uuid(void);
    memcpy(flash_uuid, get_norflash_uuid(), 16);
    for (int i = 0; i < 4; i++) {
        flash_uuid[i] ^= 0X123455AA; // 在flash的uuid基础上再叠加一层key
    }
    memcpy(test_key, flash_uuid, 16);

    printf("\n\n\n upay init new key : \n\n\n");
    put_buf(test_key, 16);

    /* printf(">>>[test]:addr = 0x%x, len = 0x%x, flash？= 0x%x\n", part_addr, part_len, sdfile_cpu_addr2flash_addr(part_addr)); */
    fclose(pfile);
    return 0;
}

int upay_read(u16 item, char *buf, int len)
{
    /* if (item > 7 || len > item_buf[item]) { */
    /*     return -1; */
    /* } */
    upay_mutex_pend();
    int rlen = 0;
    _UPAY_INFO info = {0};
    int offset = upay_search_id_info(&info, item, 1);
    printf("upay_read:0x%x,%d,%d, info:%d \n", item, len, offset, info.len);
    if ((offset == -1) || (info.len <= 0) || (info.len > len) || ((offset + sizeof(_UPAY_INFO) + len - part_addr) > part_len)) {

        rlen = -UPAY_RV_NOT_FOUND;
        goto __exit;
    }
    rlen = norflash_origin_read(buf, offset + sizeof(_UPAY_INFO), info.len);
    printf("upay_read, rlen:%d \n", rlen);
    if (info.data_crc != CRC16(buf, info.len)) {
        rlen = -UPAY_RV_NOT_FOUND;
        goto __exit;
    }
    /* y_printf(">>>[test]:before dec\n"); */
    /* put_buf(buf, rlen); */
    // dec data.
    upay_enc_dec_bank(&info, buf, info.len, 1);//解密buf
    /* y_printf(">>>[test]:after dec\n"); */
    /* put_buf(buf, rlen); */
__exit:
    upay_mutex_post();
    return rlen;
}

int upay_write(u16 item, char *idat, int len)
{
    /* if (item > 7 || len > item_buf[item]) { */
    /*     return -1; */
    /* } */
    upay_mutex_pend();
    int ret = 0;
    u16 i = upay_get_first_one(item);
    if (i >= ITEM_MAX) {
        ret = -UPAY_RV_WRITE_ERROR;
        goto __exit;
    }
    _UPAY_INFO info = {0};
    int offset = upay_search_id_info(&info, item, 0);
    /* printf(">>>[test]:offset = %d, size =%d\n",  offset, sizeof(_UPAY_INFO)); */
    offset = (offset + sizeof(_UPAY_INFO) - 1) / sizeof(_UPAY_INFO) * sizeof(_UPAY_INFO);
    //printf("upay_write:0x%x,%d,%d, info:%d \n", item, len, offset, info.len);
    if (offset == -1 || (offset + sizeof(_UPAY_INFO) + len - part_addr) > part_len) {
        ret = -UPAY_RV_WRITE_ERROR;
        goto __exit;
    }
    if (item_len[i]) {
        if (len > item_len[i]) {
            r_printf("len is bigger than last writing");
        }
        int next_offset = upay_search_next_id(offset + item_len[i]);
        if ((len > next_offset - offset) || (next_offset == -1)) {
            ret = -UPAY_RV_WRITE_ERROR;
            goto __exit;
        }
    }
    char *buf = malloc(len);
    ASSERT(buf);
    memcpy(buf, idat, len);
    // enc data.
    upay_enc_dec_bank(&info, buf, len, 0);//加密buf
    info.data_crc = CRC16(buf, len);
    info.item_id = item;
    info.len = len;
    info.head_crc = CRC16(&info.data_crc, sizeof(_UPAY_INFO) - 2);
    /* y_printf("\n >>>[test]:func = %s,line= %d\n",__FUNCTION__, __LINE__); */
    /* put_buf(&info, 32); */
    /* y_printf("\n >>>[test]:func = %s,line= %d\n",__FUNCTION__, __LINE__); */
    /* put_buf(buf, len); */

    char *tmp_buf = (char *)zalloc(4096);
    ASSERT(tmp_buf);
    offset -= part_addr;
    int rlen = norflash_origin_read(tmp_buf, part_addr, 4096);
    norflash_erase(IOCTL_ERASE_SECTOR, part_addr); //擦除4k
    memcpy(tmp_buf + offset, &info, sizeof(_UPAY_INFO));
    memcpy(tmp_buf + offset + sizeof(_UPAY_INFO), buf, len);
    int wlen = norflash_write(NULL, tmp_buf, 4096, part_addr);
    item_len[i] = len;
    if (wlen != 4096) {
        len = -UPAY_RV_WRITE_ERROR;
        item_len[i] = 0;
    }
    if (tmp_buf) {
        free(tmp_buf);
    }
    if (buf) {
        free(buf);
    }
    ret = len;
__exit:
    upay_mutex_post();
    return ret;
}

/* int upay_seek(u16 item, int addr) */
/* { */
/*     int offset = addr + upay_get_offset(item); */
/*     if (offset > part_addr) { */
/*         return -1; */
/*     } */
/*     return fseek(pfile, offset, SEEK_SET); */
/* } */


int upay_exists(u16 item)
{
    upay_mutex_pend();
    int ret =  -UPAY_RV_NOT_FOUND;
    _UPAY_INFO info = {0};
    int offset = upay_search_id_info(&info, item, 1);
    if ((offset != -1) && info.len) {
        ret = UPAY_RV_OK;
    }
    upay_mutex_post();
    return ret;
}

int upay_delete(u16 item)
{
    upay_mutex_pend();
    int ret = UPAY_RV_OK;
    /* printf(">>>[test]:upay delete!!!!!!!! item = 0x%x\n", item); */
    _UPAY_INFO info = {0};
    int offset = upay_search_id_info(&info, item, 1);
    if ((offset == -1) && !info.len) {
        ret = -UPAY_RV_NOT_FOUND;
        goto __exit;
    }
    char *tmp_buf = (char *)zalloc(4096);
    ASSERT(tmp_buf);
    offset -= part_addr;
    /* printf(">>>[test]:delete offset = %d\n", offset); */
    int rlen = norflash_origin_read(tmp_buf, part_addr, 4096);
    norflash_erase(IOCTL_ERASE_SECTOR, part_addr); //擦除4k
    memset(&info, 0, sizeof(_UPAY_INFO));
    memcpy(tmp_buf + offset, &info, sizeof(_UPAY_INFO));
    /* memcpy(tmp_buf + offset + sizeof(_UPAY_INFO), buf, len); */
    int wlen = norflash_write(NULL, tmp_buf, 4096, part_addr);
    if (tmp_buf) {
        free(tmp_buf);
    }
    if (wlen != 4096) {
        ret = -UPAY_RV_DEL_ERROR;
    }
__exit:
    upay_mutex_post();
    return ret;
}

int upay_defrag(void)
{
    upay_mutex_pend();
    int res = UPAY_RV_OK;
    char *tmp_buf = (char *)zalloc(4096);
    ASSERT(tmp_buf);

    _UPAY_INFO find_head = {0};
    int i = part_addr;
    for (; i < part_addr + part_len;) {
        norflash_origin_read((u8 *)&find_head, i, sizeof(_UPAY_INFO));
        if (find_head.head_crc == CRC16((u8 *)&find_head + 2, sizeof(_UPAY_INFO) - 2)) {
            norflash_origin_read(tmp_buf, i, find_head.len + sizeof(_UPAY_INFO));
            tmp_buf += find_head.len + sizeof(_UPAY_INFO);
            i += find_head.len;
        }
        i += sizeof(_UPAY_INFO);
    }
    /* put_buf(tmp_buf, 4096); */
    norflash_erase(IOCTL_ERASE_SECTOR, part_addr); //擦除4k
    int wlen = norflash_write(NULL, tmp_buf, 4096, part_addr);
    if (wlen != 4096) {
        res = -UPAY_RV_WRITE_ERROR;
    }
    if (tmp_buf) {
        free(tmp_buf);
    }
    upay_mutex_post();
    return res;
}


void upay_test_demo()
{
    char tbuf[126] = {0};
    upay_init("UPAY");
    int item = 1;
#define test_max 8
    for (; item  < test_max; item += 1) {
        memset(tbuf, item, sizeof(tbuf));
        int wlen = upay_write(item, tbuf, sizeof(tbuf));
        r_printf(">>>[test]:wlen = %d\n", wlen);
    }
    for (item = 1; item  < test_max; item ++) {
        memset(tbuf, 0x50 + item, sizeof(tbuf));
        if (item % 2) {
            upay_delete(item);
            r_printf(">>>[test]:delete item = %d\n", item);
            memset(tbuf, 0xa0 + item, sizeof(tbuf));
        }
        if (item != test_max - 1) {
            upay_write(item, tbuf, sizeof(tbuf));
        }
    }
    for (item = 1; item  < test_max; item += 1) {
        int ret = upay_exists(item);
        r_printf(">>>[test]:item  exists is %d \n", ret);
    }
    for (int item = 1; item  < test_max; item += 1) {
        memset(tbuf, 0, sizeof(tbuf));
        int rlen = upay_read(item, tbuf, sizeof(tbuf));
        r_printf(">>>[test]:rlen = %d\n", rlen);
    }
}
