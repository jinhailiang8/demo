#include "typedef.h"
#include "asm/clock.h"
#include "asm/adc_api.h"
#include "timer.h"
#include "init.h"
#include "asm/efuse.h"
#include "irq.h"
#include "asm/power/p33.h"
#include "asm/power/p11.h"
#include "asm/power_interface.h"
#include "jiffies.h"
#include "app_config.h"
#include "syscfg_id.h"
#include "system/includes.h"
#include "adc_dtemp_alog.h"


//***************************
// dtemp pll trim************
//***************************

#define CHECK_TEMPERATURE_CYCLE             (500)   //
#define BTOSC_TEMPERATURE_THRESHOLD         (3 * 40) //

#define     APP_IO_DEBUG_0(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     APP_IO_DEBUG_1(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}



extern const int config_bt_temperature_pll_trim;

extern void get_bta_pll_midbank_temp(void);   //ok

static u8 dtemp_data_change = 0;
static u8 is_pll_trim_active = 0;

static void btpll_trim_main()
{
    /* printf("midbank_trim>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n"); */
    local_irq_disable();
    is_pll_trim_active = 1;
    local_irq_enable();

    get_bta_pll_midbank_temp();

    local_irq_disable();
    is_pll_trim_active = 0;
    local_irq_enable();

}
void check_pll_trim_condition(TempSensor *tem, u8 en)
{
    TempSensor *data = get_tempsensor_pivr();
    int vaild = data->valid;
    int stable = data->stable;
    /* printf("vaild:%d,stable:%d\n",vaild,stable); */
    if (vaild && stable) {
        //比较

        if ((data->ref > data->output) && ((data->ref - data->output) > BTOSC_TEMPERATURE_THRESHOLD)) {
            tempsensor_update_ref(data, data->ref - BTOSC_TEMPERATURE_THRESHOLD);
            dtemp_data_change = 1;
        } else if ((data->ref < data->output) && ((data->output - data->ref) > BTOSC_TEMPERATURE_THRESHOLD)) {
            tempsensor_update_ref(data, data->ref + BTOSC_TEMPERATURE_THRESHOLD);
            dtemp_data_change = 1;
        }

    }
    if (dtemp_data_change) {

        int msg[3] = {0};
        msg[0] = (int)btpll_trim_main;
        msg[1] = 1;
        msg[2] = 0;
        int ret = os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);
        if (ret == 0) {

            dtemp_data_change = 0;
        }


    }
}
static void pll_trim_timer_handler(void *priv)
{
    if (!config_bt_temperature_pll_trim) {
        return;
    }

    u16 dtemp_voltage = adc_get_voltage(AD_CH_DTEMP);
    if (dtemp_voltage <= 0) { //LVD DISABLE 的情况下 电压值会为0 异常
        return ;
    }

    tempsensor_process(get_tempsensor_pivr(), dtemp_voltage);

    check_pll_trim_condition(get_tempsensor_pivr(), 1);
}

void temp_pll_trim_init(void)
{
    if (!config_bt_temperature_pll_trim) {

        printf("BTPLL TRIM DISABLE>>>n");
        return;
    }
    printf("BTPLL TRIM EN >>>n");
    tempsensor_init(get_tempsensor_pivr());
    sys_s_hi_timer_add(NULL, pll_trim_timer_handler, CHECK_TEMPERATURE_CYCLE);
}


static u8 pll_trim_idle_query(void)
{
    return (!is_pll_trim_active);
}

REGISTER_LP_TARGET(plll_trim_lp_target) = {
    .name = "pll_trim",
    .is_idle = pll_trim_idle_query,
};


