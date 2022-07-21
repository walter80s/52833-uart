
#include "app_timer.h"
#include "nrf_pwr_mgmt.h"
#include "app_scheduler.h"
#include "mt_ble_cfg.h"
#include "mt_ble_adv.h"
#include "mt_serial.h"
#include "mt_wdt.h"
#include "mt_logic.h"

#define NRF_LOG_MODULE_NAME main
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

#define SCHED_MAX_EVENT_DATA_SIZE 8  /**< scheduler 事件数据的最大值. */
#define SCHED_QUEUE_SIZE          20 /**< scheduler 队列的长度. */

/**
 * @brief 空闲处理
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false){
        nrf_pwr_mgmt_run();
    }
}

/**
 * @brief 获取复位原因.
 */
static void reset_reason_get(void)
{
    NRF_LOG_INFO("reset reason is: %08x.", NRF_POWER->RESETREAS);
    NRF_POWER->RESETREAS = 0xFFFFFFFF; /* 清除复位原因寄存器 */
}

/**@brief log 模块初始化.
 */
static void log_init(void)
{
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

int main(void)
{
    log_init();
    reset_reason_get();
    APP_ERROR_CHECK(app_timer_init());
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    APP_ERROR_CHECK(nrf_pwr_mgmt_init());
    mt_serial_init();
    mt_ble_config_init();
    mt_wdt_init();
    mt_feed_wdt();
    mt_logic_init();

    NRF_LOG_INFO("52833 uart.");

    for (;;){
        app_sched_execute();
        idle_state_handle();
    }
}

