#ifndef UPAY_VENDOR_H
#define UPAY_VENDOR_H

#include "upay_common.h"

typedef enum {
    // private key
    UPAY_ITEM_PRIVATE_KEY = 0x01 << 0,
    // shared key based on private key and server's public key
    UPAY_ITEM_SHARE_KEY   = 0x01 << 1,
    // encrypted seed from server side
    UPAY_ITEM_SEED        = 0x01 << 2,
    // time diff between iot client and server
    UPAY_ITEM_TIMEDIFF    = 0x01 << 3,
    // nick name for user
    UPAY_ITEM_NICKNAME    = 0x01 << 4,
    // logon ID for user
    UPAY_ITEM_LOGONID     = 0x01 << 5,
    // binding flag, see binding_status_e for value
    UPAY_ITEM_BINDFLAG    = 0x01 << 6,
} UPAY_storage_item;

/////////////////////////////////////////////
////安全存储:读取
////注:为防止存储破坏，需要加上校验值
/// 返回值：
/// RV_OK：读取成功
/// RV_READ_ERROR：读取失败
/// RV_NOT_FOUND：对应数据找不到
/////////////////////////////////////////////
EXTERNC upay_retval_t upay_storage_read(UPAY_storage_item item,
                                        uint8_t *buf_content, uint32_t *len_content);

/////////////////////////////////////////////
////安全存储:写入
////注: 必须实现设备相关加解密，即写入到A设备
////的内容，即使拷贝到B设备，也无法正常使用
////此外为防止存储破坏，需要加上校验值
/// 返回值：
/// RV_OK：写入成功
/// RV_WRITE_ERROR：写入失败
/////////////////////////////////////////////
EXTERNC upay_retval_t upay_storage_write(UPAY_storage_item item,
        const uint8_t *buf_content, uint32_t len_content);

/////////////////////////////////////////////
////安全存储:删除(需要支持一次删除多个item,
////如:UPAY_ITEM_PRIVATE_KEY | UPAY_ITEM_SEED)
/// 返回值：
/// RV_OK：删除成功
/// RV_DEL_ERROR：删除失败
/////////////////////////////////////////////
EXTERNC upay_retval_t upay_storage_delete(uint32_t items);


/////////////////////////////////////////////
////安全存储:判断是否存在文件
/// 返回值：
/// RV_OK：存在
/// RV_NOT_FOUND：不存在
/////////////////////////////////////////////
EXTERNC upay_retval_t upay_storage_exists(UPAY_storage_item item);


/////////////////////////////////////////////
////获取当前系统时间戳
/////////////////////////////////////////////
EXTERNC uint32_t upay_get_timestamp(void);

/*BLE写接口:
 * @param [in] data 待发送数据的指针
 * @param [in] len  待发送数据的大小
 * @return void
 */
EXTERNC void upay_ble_write(uint8_t *data, uint16_t len);

/////////////////////////////////////////////
////获取设备ID(以冒号分割的16进制mac地址)
////要求内容以‘\0’结尾且长度不包含'\0'
/////////////////////////////////////////////
EXTERNC upay_retval_t upay_get_deviceid(uint8_t *buf_did, uint32_t *len_did);

/////////////////////////////////////////////
////获取设备SN(厂商印刷在卡片上的设备序列号)
////长度不超过32个字符，只能包含大小写字母、数字、下划线
////要求内容以‘\0’结尾且长度不包含'\0'
////仅卡片类产品且有SN在小程序显示需求的厂商实现
////其他厂商请输出buf_sn=""(空字符串)，len_sn=0
/////////////////////////////////////////////
EXTERNC upay_retval_t upay_get_sn(uint8_t *buf_sn, uint32_t *len_sn);

/////////////////////////////////////////////
////日志信息输出接口
/////////////////////////////////////////////
typedef enum {
    UPAY_LEVEL_FATAL = 0x01,
    UPAY_LEVEL_ERRO,
    UPAY_LEVEL_WARN,
    UPAY_LEVEL_INFO,
    UPAY_LEVEL_DBG,
} UPAY_log_level;

/////////////////////////////////////////////////////////////////////////////////////
// 由于有些设备上不支持输出动态string, 所以没有定义成变长参数，
// 只会输出一些关键的trace log, 便于发现问题时快速定位
// 注: format参数中可能含有静态string, 实现时不可忽略
/////////////////////////////////////////////////////////////////////////////////////
EXTERNC void upay_log(UPAY_log_level, const char *format, int32_t value);
EXTERNC void upay_log_ext(const char *format, ...);
#endif
