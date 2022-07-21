
#include "nrf_soc.h"

#define MT_WDT_TIMEOUT 10000 /* 单位 ms  */

/**
 * @brief 看门狗模块初始化
 */
void mt_wdt_init(void)
{
    NRF_WDT->CONFIG      = 0x09; /* 在 sleep 和 halt 下都可运行 */
    NRF_WDT->CRV         = (MT_WDT_TIMEOUT * 32768 / 1000) - 1;
    NRF_WDT->RREN        = 0x01; /* RR[0] */
    NRF_WDT->TASKS_START = 1;    /* 启动看门狗 */
}

/**
 * @brief 喂狗
 */
void mt_feed_wdt(void)
{
    NRF_WDT->RR[0] = 0x6E524635UL;
}
