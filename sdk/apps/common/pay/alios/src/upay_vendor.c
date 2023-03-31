//该文件是需要厂商实现的底层接口
#include "upay.h"
#include "upay_vendor.h"
#include "custom_cfg.h"
#include "le_smartbox_adv.h"
#include "timestamp.h"

#define vendor_printf(...) printf(__VA_ARGS__)

#define MY_DEBUG(...)
#define MY_DEBUG_BUF(x,y)
/* #define MY_DEBUG	printf */
/* #define MY_DEBUG_BUF	put_buf */

#define CST_OFFSET_TIME			(8*60*60)	// 北京时间时差

#define UPAY_DATA_BUF_TEST		0

#define TEST_BUF_ITEM_MAX		7
#if UPAY_DATA_BUF_TEST
static u32 test_buf_len[TEST_BUF_ITEM_MAX] = {0};
static u32 test_buf[TEST_BUF_ITEM_MAX][512] = {0};
#endif

extern int le_controller_get_mac(void *addr);
extern void bt_addr2string(u8 *addr, u8 *buf);

/////////////////////////////////////////////
////安全存储:读取
////注:为防止存储破坏，需要加上校验值
/// 返回值：
/// UPAY_RV_OK：读取成功
/// UPAY_RV_READ_ERROR：读取失败
/// UPAY_RV_NOT_FOUND：对应数据找不到
/////////////////////////////////////////////
upay_retval_t upay_storage_read(UPAY_storage_item item, uint8_t *buf_content, uint32_t *len_content)
{
    // 需要厂商实现
    vendor_printf("%s-%d called\n", __FUNCTION__, __LINE__);
    MY_DEBUG("item:0x%x, len:%d \n", item, *len_content);
#if UPAY_DATA_BUF_TEST
    for (int i = 0; i < TEST_BUF_ITEM_MAX; i++) {
        if (item & BIT(i)) {
            if (test_buf_len[i]) {
                memcpy(buf_content, test_buf[i], test_buf_len[i]);
                *len_content = test_buf_len[i];
                MY_DEBUG("read_len[%d]:%d \n", i, *len_content);
                MY_DEBUG_BUF(buf_content, *len_content);
                return UPAY_RV_OK;
            }
            break;
        }
    }
    MY_DEBUG("no find \n");
    return UPAY_RV_NOT_FOUND;
#else
    int len = *len_content;
    for (int i = 0; i < TEST_BUF_ITEM_MAX; i++) {
        if (item & BIT(i)) {
            int rlen = upay_read((u16)item, buf_content, len);
            MY_DEBUG("read_len[%d]:%d \n", i, rlen);
            MY_DEBUG_BUF(buf_content, rlen);
            if ((rlen <= 0) || (rlen > len)) {
                MY_DEBUG("no find \n");
                return UPAY_RV_NOT_FOUND;
            }
            *len_content = rlen;
            break;
        }
    }
    return UPAY_RV_OK;
#endif
    return UPAY_RV_OK;
}

/////////////////////////////////////////////
////安全存储:写入
////注: 必须实现设备相关加解密，即写入到A设备
////的内容，即使拷贝到B设备，也无法正常使用
////此外为防止存储破坏，需要加上校验值
/// 返回值：
/// UPAY_RV_OK：写入成功
/// UPAY_RV_WRITE_ERROR：写入失败
/////////////////////////////////////////////
upay_retval_t upay_storage_write(UPAY_storage_item item, const uint8_t *buf_content, uint32_t len_content)
{
    // 需要厂商实现
    vendor_printf("%s-%d called\n", __FUNCTION__, __LINE__);
    MY_DEBUG("item:0x%x, len:%d \n", item, len_content);
#if UPAY_DATA_BUF_TEST
    for (int i = 0; i < TEST_BUF_ITEM_MAX; i++) {
        if (item & BIT(i)) {
            memcpy(test_buf[i], buf_content, len_content);
            test_buf_len[i] = len_content;
            MY_DEBUG("write_len[%d]:%d \n", i, test_buf_len[i]);
            MY_DEBUG_BUF(test_buf[i], test_buf_len[i]);
            break;
        }
    }
#else
    int retry_time = 4;
    int ret;
    int len = len_content;
    for (int i = 0; i < TEST_BUF_ITEM_MAX; i++) {
        if (item & BIT(i)) {
            for (int j = 0; j < retry_time; j++) {
                int wlen = upay_write((u16)item, buf_content, len);
                MY_DEBUG("write rlen:%d \n", wlen);
                if (wlen != len) {
                    if (j < retry_time - 1) {
                        if (j == retry_time - 1) { //重复三次无法写入，进入碎片整理，再写一次。
                            if (upay_defrag()) {
                                MY_DEBUG("upay_defrag err \n");
                                return UPAY_RV_WRITE_ERROR;
                            }
                        }
                        continue;
                    }
                    MY_DEBUG("upay_write err \n");
                    return UPAY_RV_WRITE_ERROR;
                }
                MY_DEBUG("write_len[%d]:%d \n", i, len);
                MY_DEBUG_BUF(buf_content, len);
                return UPAY_RV_OK;
            }
        }
    }
    return UPAY_RV_OK;
#endif
    return UPAY_RV_OK;
}

/////////////////////////////////////////////
////安全存储:删除(需要支持一次删除多个item,
////如:ALIPAY_ITEM_PRIVATE_KEY | ALIPAY_ITEM_SEED)
/// 返回值：
/// UPAY_RV_OK：删除成功
/// UPAY_RV_DEL_ERROR：删除失败
/////////////////////////////////////////////
upay_retval_t upay_storage_delete(uint32_t items)
{
    // 需要厂商实现
    vendor_printf("%s-%d called\n", __FUNCTION__, __LINE__);
    MY_DEBUG("items:0x%x \n", items);
#if UPAY_DATA_BUF_TEST
    for (int i = 0; i < TEST_BUF_ITEM_MAX; i++) {
        if (items & BIT(i)) {
            test_buf_len[i] = 0;
            MY_DEBUG("del:%d \n", i);
        }
    }
#else
    for (int i = 0; i < TEST_BUF_ITEM_MAX; i++) {
        if (items & BIT(i)) {
            int ret = upay_delete((u16)BIT(i));
            MY_DEBUG(">>>[test]:ret = %d\n", ret);
        }
    }
#endif
    return UPAY_RV_OK;
}


/////////////////////////////////////////////
////安全存储:判断是否存在文件
/// 返回值：
/// UPAY_RV_OK：存在
/// UPAY_RV_NOT_FOUND：不存在
/////////////////////////////////////////////
upay_retval_t upay_storage_exists(UPAY_storage_item item)
{
    // 需要厂商实现
    vendor_printf("%s-%d called\n", __FUNCTION__, __LINE__);
    MY_DEBUG("item:0x%x \n", item);
#if UPAY_DATA_BUF_TEST
    for (int i = 0; i < TEST_BUF_ITEM_MAX; i++) {
        if (item & BIT(i)) {
            if (test_buf_len[i]) {
                MY_DEBUG("find \n");
                return UPAY_RV_OK;
            }
        }
    }
    MY_DEBUG("no find \n");
    return UPAY_RV_NOT_FOUND;
#else
    for (int i = 0; i < TEST_BUF_ITEM_MAX; i++) {
        if (item & BIT(i)) {
            int ret = upay_exists((u16)item);
            if (ret) {
                MY_DEBUG("no find \n");
                return UPAY_RV_NOT_FOUND;
            }
            MY_DEBUG("find \n");
            return UPAY_RV_OK;
        }
    }
#endif
    return UPAY_RV_OK;
}


/////////////////////////////////////////////
////获取当前系统时间戳
/////////////////////////////////////////////
uint32_t upay_get_timestamp(void)
{
    // 需要厂商实现
    struct sys_time time;
    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        MY_DEBUG("upay_get_timestamp err!!!\n");
        return UPAY_RV_IO_ERROR;
    }
    dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)&time);
    dev_close(fd);
    uint32_t utime = timestamp_mytime_2_utc_sec(&time) - CST_OFFSET_TIME;
    MY_DEBUG("upay_get_timestamp: %d,%d,%d,%d,%d,%d : %d,0x%x \n", time.year, time.month, time.day, time.hour, time.min, time.sec, utime, utime);
    return utime;
}

/*BLE写接口:
 * @param [in] data 待发送数据的指针
 * @param [in] len  待发送数据的大小
 * @return void
 */
void upay_ble_write(uint8_t *data, uint16_t len)
{
    // 需要厂商实现
    upay_ble_send_data(data, len);
}

/////////////////////////////////////////////
////获取设备ID(以冒号分割的16进制mac地址)
////要求内容以‘\0’结尾且长度不包含'\0'
/////////////////////////////////////////////
upay_retval_t upay_get_deviceid(uint8_t *buf_did, uint32_t *len_did)
{
    // 需要厂商实现
    u8 mac[6];
    le_controller_get_mac(mac);
    bt_addr2string(mac, buf_did);
    *len_did = strlen(buf_did);
    MY_DEBUG("get mac:%d, %s \n", *len_did, buf_did);
    return UPAY_RV_OK;
}

/////////////////////////////////////////////
////获取设备SN(厂商印刷在卡片上的设备序列号)
////长度不超过32个字符，只能包含大小写字母、数字、下划线
////要求内容以‘\0’结尾且长度不包含'\0'
////仅卡片类产品且有SN在小程序显示需求的厂商实现
////其他厂商请输出buf_sn=""(空字符串)，len_sn=0
/////////////////////////////////////////////
upay_retval_t upay_get_sn(uint8_t *buf_sn, uint32_t *len_sn)
{
    // 需要厂商实现
    extern u32 get_product_serial_num(u8 * buf, u16 len);
    u32 rlen = get_product_serial_num(buf_sn, 31);
    buf_sn[rlen] = '\0';
    *len_sn = rlen;
    MY_DEBUG("get sn:%d, %s \n", *len_sn, buf_sn);
    return UPAY_RV_OK;
}

/////////////////////////////////////////////////////////////////////////////////////
// 由于有些设备上不支持输出动态string, 所以没有定义成变长参数，
// 只会输出一些关键的trace log, 便于发现问题时快速定位
// 注: format参数中可能含有静态string, 实现时不可忽略
/////////////////////////////////////////////////////////////////////////////////////
void upay_log(UPAY_log_level level, const char *format, int32_t value)
{
    // 需要厂商实现
    printf(format, value);
}
void upay_log_ext(const char *format, ...)
{
    // 需要厂商实现
    va_list args;
    va_start(args, format);
    print(NULL, 0, format, args);

    /* puts("\n\n>>>>>>>>> upaytest >>>>>>>>\n\n"); */
    /* puts(format); */
    /* putchar('\n'); */
    /* putchar('\n'); */
}

#if 0
// 有些支付宝库需要实现以下函数
int ALIPAY_get_protocType(void)
{
    return 2;
}
int ALIPAY_get_productType(void)
{
    return UPAY_WATCH;
}

uint8_t *ALIPAY_get_productModel(void)
{
    return TCFG_PAY_ALIOS_PRODUCT_MODEL;
}

uint8_t *ALIPAY_get_companyName(void)
{
    return "XXX"//; // 需要填写厂商代号。一般就是TCFG_PAY_ALIOS_PRODUCT_MODEL的前几个字符
}
#endif


