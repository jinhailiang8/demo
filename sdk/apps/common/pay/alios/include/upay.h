
#ifndef UPAY_H
#define UPAY_H
#include "upay_common.h"

//儿童类产品请选择亲子类型。其他产品同意使用前面3个
typedef enum {
    UPAY_WATCH,     //手表
    UPAY_BAND,      //手环
    UPAY_CARD,      //卡片
    UPAY_QZWT,      //亲子手表
    UPAY_QZBD,      //亲子手环
    UPAY_QZCRD,     //亲子卡片
} upay_product_type;

/**
 * 初始化SDK，接口调用之后绑定状态为STATUS_START_BINDING
 * @param [in]  type    产品的类型，厂商根据设备的真实产品类型填写，参考upay_product_type
 * @param [in]  upay_product_model    字符串格式的设备型号信息，长度必须<=18，建议<=15
 *              例如设备厂商支付宝的W100型号的设备，应填写："UPAY_W100";
 *              特别注意：
 *              任何厂商在接入新型号设备时，请务必使用新的设备型号命名，确保每个机型都有不同的型号名（参考文档10.1章节）！
 *              每一款设备接入SDK后，在上线之前都需要将设备型号命名发给支付宝技术人员审核，审核通过后才允许上线。
 *              不然将可能产生严重的后果！如果由于厂商未按要求命名设备型号或未经审核私自上线造成严重后果的，厂商全责！
 *              该设备将会被拉入黑名单！情节严重的，将视为违规接入，取消合作关系，并根据法务合同追责。
 * @param [in]  upay_ptc_type    协议类型。厂商传入数字 2 即可

 * @return  RV_OK：初始化成功。如果返回值不为成功，说明入参不正确，请勿使用SDK
 *          RV_WRONG_PARAM：参数异常
 */
EXTERNC upay_retval_t upay_env_init(upay_product_type type, uint8_t *upay_product_model, uint8_t upay_ptc_type);

/**
 * 清除绑定状态数据
 * 退出绑定界面时调用
 */
EXTERNC void upay_env_deinit(void);

/**
 * 获取绑定码
 * @param [out] binding_str     存放绑定码的指针
 * @param [out] len_binding_str 绑定码数据大小，传入时为binding_str buff大小
 * @return  RV_OK：    获取成功
 *          RV_BINDING_STR_TOO_LONG：数据size超过限制，请减少upay_product_model的长度
 */
EXTERNC upay_retval_t upay_get_binding_string(uint8_t *binding_str, uint32_t *len_binding_str);

/**
 * 获取支付码，支付码为18或19位纯数字字符串，条形码编码和显示时应注意
 * @param [out] paycode     存放支付码的指针
 * @param [out] len_paycode 支付码数据大小，传入时为paycode buff大小
 * @return  RV_OK：    获取成功
 *          其他value：获取失败
 */
EXTERNC upay_retval_t upay_get_paycode(uint8_t *paycode, uint32_t *len_paycode);

/**
 * 支付功能解绑，清除绑定数据
 * @return  RV_OK：    成功
 *          其他value：失败  频繁失败说明芯片可能出错，建议禁止支付功能
 */
EXTERNC upay_retval_t upay_unbinding(void);

/**
 * 获取绑定状态
 * @param [in] void
 * @return 返回绑定状态
 */
EXTERNC upay_binding_status_e upay_get_binding_status(void);

/**
 * 从Flash读取存储的logon_ID
 * @param [out] id 存放数据的指针
 * @param [out] len_id数据大小，最大读取不超过传入这个值的数据大小
 * @return  RV_OK：读取成功
 *          RV_NOT_FOUND：未发现该存储项
 */
EXTERNC upay_retval_t upay_get_logon_ID(uint8_t *id, uint32_t *len_id);

/**
 * 从Flash读取存储的nick_name
 * @param [out] nick_name 存放数据的指针
 * @param [out] len_nick_name数据大小，最大读取不超过传入这个值的数据大小
 * @return  RV_OK：读取成功
 *          RV_NOT_FOUND：未发现该存储项
 */
EXTERNC upay_retval_t upay_get_nick_name(uint8_t *nick_name, uint32_t *len_nick_name);

/**
 * BLE数据接收的CallBack函数
 * @param [out] data 存放数据的指针
 * @param [out] len 收到的数据大小
 * @return void
 */
EXTERNC void upay_recv_data_handle(const uint8_t *data, uint16_t len);

/**
 * Vendor软件如果有对时逻辑，设备与服务端对时成功之后应该调用本接口通知SDK
 * （此处对时特指将本地时间和网络端时间对齐。对齐之后通知SDK即可。）
 */
EXTERNC void upay_vendor_sync_time_done(void);
#endif