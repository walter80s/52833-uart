/* Copyright (C) Shenzhen Minew Technologies Co., Ltd
   All rights reserved. */

#include "nrf.h"
#include "nrf_gpio.h"
#include "nordic_common.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "app_error.h"
#include "app_scheduler.h"
#include "mt_logic.h"
#include "mt_serial.h"
#include "mt_ble_cfg.h"
#include "mt_ble_adv.h"
#include "mt_wdt.h"
#include "mt_cst_protocol.h"

#define NRF_LOG_MODULE_NAME logic
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define WDT_FEED_TIME APP_TIMER_TICKS(4000) /**< 看门狗喂狗间隔 */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; /**< 连接句柄. */
static uint8_t  m_dev_power   = DEV_POWERON;             /**< 设备开关机状态. */

APP_TIMER_DEF(m_wdt_timer); /**< 看门狗喂狗定时器. */

/**
 * @brief 看门狗喂狗
 */
static void wdt_feed_sched_handler(void *p_ctx, uint16_t len)
{
    mt_feed_wdt();
    nrf_delay_us(150);
}

/**
 * @brief 看门狗喂狗定时器超时处理函数.
 */
static void wdt_timeout_handler(void *p_context)
{
    APP_ERROR_CHECK(app_sched_event_put(NULL, 0, wdt_feed_sched_handler));
}

/**
 * @brief 开机处理函数.
 */
static void dev_poweron_hdl(void *p_ctx, uint16_t len)
{
    NRF_LOG_INFO("dev_poweron_hdl");

    m_dev_power = DEV_POWERON;
    ble_adv_start();
    mt_serial_ctrl_handler(true);
}

/**
 * @nrief GATT 事件处理.
 */
static void gatt_event_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) &&
        (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED)) {
        NRF_LOG_INFO("MTU update %d", p_evt->params.att_mtu_effective - 3);
    }
}

/**
 * @brief 串口模块上报的事件处理函数.
 *
 * @param[in] p_evt 指向串口模块上报的事件.
 */
static void serial_evt_handler(serial_evt_t *p_evt)
{
    ASSERT(p_evt != NULL);

    switch (p_evt->evt_id) {
        case SERIAL_EVT_RX_OVER:
            uart_receive_handler(p_evt->p_data,p_evt->len);
            break;

        default:
            break;
    }
}

/**
 * @brief 处理来自协议栈的BLE事件回调函数.
 *
 * @param[in] p_ble_evt BLE 协议栈事件
 * @param[in] p_context 未使用
 */
static void logic_ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
    ble_gap_evt_t const *p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("conn ok");
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            mt_serial_flush();
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            mt_serial_flush();
            NRF_LOG_INFO("disconnected reason %x\r\n",
                         p_ble_evt->evt.gap_evt.params.disconnected.reason);
            if (m_dev_power == DEV_POWEROFF) {
                break;
            }
            ble_adv_start();
            break;

        case BLE_GAP_EVT_TIMEOUT:
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN) {}
            break;

        default:
            break;
    }
}

NRF_SDH_BLE_OBSERVER(m_ble_observer, 3, logic_ble_evt_handler, NULL);

void mt_logic_init(void)
{
    mt_ble_gatt_register(gatt_event_handler);
    mt_serial_hdl_register(serial_evt_handler);
    mt_user_protocol_init();

    APP_ERROR_CHECK(app_timer_create(&m_wdt_timer, APP_TIMER_MODE_REPEATED, wdt_timeout_handler));
    APP_ERROR_CHECK(app_timer_start(m_wdt_timer, WDT_FEED_TIME, NULL));
    APP_ERROR_CHECK(app_sched_event_put(NULL, 0, dev_poweron_hdl));
}