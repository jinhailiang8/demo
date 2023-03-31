#include "upay.h"
#include "app_config.h"




void upay_binding_test()
{
    int timeout = 0; //100s
    printf("Start binding...\n");
    printf("Before init ,status is %02X\n", upay_get_binding_status());
    upay_env_init(UPAY_WATCH, TCFG_PAY_ALIOS_PRODUCT_MODEL, 2);    //环境初始化，此时状态应该为STATUS_START_BINDING
    printf("After init ,status is %02X\n", upay_get_binding_status());
    //upay_ble_service_enable();    //打开Alipay BLE通信    @iot
    uint8_t device_info[57] = {0};
    uint32_t lenth_info = 0;
    upay_get_binding_string(device_info, &lenth_info);    //获取绑定码
    printf("need generage qrcode: device_info[%d]: %s\n", lenth_info, device_info);
    //////////二维码编码和显示////////////////
    //    uint8_t bitdata[QR_MAX_BITDATA];
    //    int side = qr_encode(QR_LEVEL_L, 0, device_info, lenth_info, bitdata);
    //    printf("Qrcode side length:%d\n", side);
    //    alipay_showQR(bitdata, side, 14, 14, 5);
    /////////////绑定状态轮询///////////////
    while (!timeout) {
        int status = upay_get_binding_status();
        // printf("quiry once, status is %02X now\n", status);
        if (UPAY_STATUS_BINDING_OK == status) {
            printf("Bind OK!\n");
            break;
        }
        if (UPAY_STATUS_BINDING_FAIL == status) {
            printf("Bind FAIL!\n");
            break;
        }
        os_time_dly(2);//vTaskDelay(2000);
    }
    upay_env_deinit();    //注销Alipay绑定环境
    printf("after deinit ,status is %02X\n", upay_get_binding_status());
    printf("End...\n ");
}

