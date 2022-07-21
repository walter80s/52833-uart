/* Copyright (C) Shenzhen Minew Technologies Co., Ltd
   All rights reserved. */

#include "mt_sw_mutex.h"
#include "mt_param_def.h"
#include "mt_serial.h"
#include "app_uart.h"
#include "app_fifo.h"
#include "app_timer.h"

#define NRF_LOG_MODULE_NAME serial
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define APP_UART_FIFO_TX_SIZE 128                 /* APP_UART 发送 FIFO 长度. */
#define APP_UART_FIFO_RX_SIZE 128                 /* APP_UART 接收 FIFO 长度. */
#define SERIAL_RX_OVER_TIME   APP_TIMER_TICKS(12) /**< 串口接收数据完成延时 */

/* clang-format off */
static uint8_t    m_uart_rx_buf[SERIAL_RX_FIFO_SIZE]; /**< 存放串口接收数据的内存. */
static uint8_t    m_uart_tx_buf[SERIAL_TX_FIFO_SIZE]; /**< 存放发送接收数据的内存. */
static uint8_t    m_tmp_buf[SERIAL_RX_FIFO_SIZE];     /**< 中转数据的内存. */
static app_fifo_t m_uart_rx_fifo;                     /**< 存放串口接收数据的 fifo. */
static app_fifo_t m_uart_tx_fifo;                     /**< 存放串口发送数据的 fifo. */

static app_uart_comm_params_t serial_cfg = DEF_SERIAL_CFG; /**< 串口的配置参数 */

static serial_callback_t m_serial_callback;         /**< 串口模块回调函数. */
static uint32_t          m_rx_cnt;                  /**< 单位时间内接收数据的个数 */
static uint8_t           m_rx_timer    = 1;         /**< 互斥锁用于控制接收完成定时器的开启 */
static bool              m_uart_state  = false;  /**< 指示当前串口状态开启还是关闭  */
static bool              m_is_sending  = false;     /**< 当前串口是否正在发送数据. */
/* clang-format on */

APP_TIMER_DEF(m_rx_over_timer); /**< 串口接收数据完成检测定时器 */

/**
 * @brief uart 开始接收处理函数
 */
static void uart_rx_start_hdl(void)
{
    MT_SW_MUTEX_T(&m_rx_timer);
    m_rx_cnt = 0;
    APP_ERROR_CHECK(app_timer_start(m_rx_over_timer, SERIAL_RX_OVER_TIME, NULL));
    NRF_LOG_INFO("rx data start");
}

/**
 * @brief 处理 app_uart 上报的事件.
 */
static void mt_uart_event_handle(app_uart_evt_t *p_event)
{
    uint8_t    tmp_data = 0;
    ret_code_t err_code;
    switch (p_event->evt_type) {
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&tmp_data));
            uart_rx_start_hdl();
            if ((app_fifo_usage_get(&m_uart_rx_fifo) + 1) <= SERIAL_RX_FIFO_SIZE) {
                UNUSED_VARIABLE(app_fifo_put(&m_uart_rx_fifo, tmp_data));
            }
            m_rx_cnt++;
            break;

        case APP_UART_COMMUNICATION_ERROR:
            NRF_LOG_WARNING("uart buadrate error");
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        case APP_UART_TX_EMPTY:
            err_code = app_fifo_get(&m_uart_tx_fifo, &tmp_data);
            if (err_code == NRF_ERROR_NOT_FOUND) {
                m_is_sending = false;
                return;
            } else {
                APP_ERROR_CHECK(err_code);
            }
            APP_ERROR_CHECK(app_uart_put(tmp_data));
            break;

        default:
            break;
    }
}

uint32_t mt_serial_put(uint8_t *p_data, uint16_t len)
{
    ret_code_t err_code;
    uint8_t    byte;

    if (m_uart_state == false) {
        return NRF_ERROR_INVALID_STATE;
    }

    uint32_t tmp_len = app_fifo_usage_get(&m_uart_tx_fifo);
    tmp_len  = (tmp_len + len) > SERIAL_TX_FIFO_SIZE ? (SERIAL_TX_FIFO_SIZE - tmp_len) : len;
    err_code = app_fifo_write(&m_uart_tx_fifo, p_data, &tmp_len);

    if (m_is_sending == false) {
        m_is_sending = true;
        APP_ERROR_CHECK(app_fifo_get(&m_uart_tx_fifo, &byte));
        APP_ERROR_CHECK(app_uart_put(byte));
    }

    return err_code;
}

/**
 * @brief 判断接收是否完成定时器超时处理
 */
static void rx_over_timeout_handler(void *p_context)
{
    serial_evt_t evt;
    /* 过去这段时间还有数据接收 */
    if (m_rx_cnt != 0) {
        m_rx_cnt = 0;
    }
    /* 过去这段事件没有数据接收,表示数据接收完成 */
    else {
        APP_ERROR_CHECK(app_timer_stop(m_rx_over_timer));
        MT_SW_MUTEX_G(&m_rx_timer);
        m_rx_cnt = 0;
        evt.evt_id = SERIAL_EVT_RX_OVER;
        evt.len    = SERIAL_RX_FIFO_SIZE;
        memset(m_tmp_buf,0,sizeof(m_tmp_buf));
        app_fifo_read(&m_uart_rx_fifo, m_tmp_buf, &evt.len);
        evt.p_data = m_tmp_buf; 
        if (m_serial_callback) {
            m_serial_callback(&evt);
        }        
        NRF_LOG_INFO("rxover OK");
    }
}

void mt_serial_flush(void)
{
    NRF_LOG_INFO("serial flush");
    APP_ERROR_CHECK(app_timer_stop(m_rx_over_timer));

    app_fifo_flush(&m_uart_rx_fifo);
    app_fifo_flush(&m_uart_tx_fifo);
    app_uart_flush();
}

bool mt_is_serail_tx_complete(void)
{
    return !m_is_sending;
}

bool mt_is_serial_enabled(void)
{
    return m_uart_state;
}

void mt_serial_hdl_register(serial_callback_t cb)
{
    m_serial_callback = cb;
}

void mt_serial_ctrl_handler(bool ctrl)
{
    if ((ctrl == false) && m_uart_state) {
        app_uart_close();
        APP_ERROR_CHECK(sd_clock_hfclk_release());
        m_uart_state = false;
        NRF_LOG_INFO("uart disabled");
    } else if ((ctrl == true) && (m_uart_state == false)) {
        ret_code_t err_code;
        APP_UART_FIFO_INIT(&serial_cfg,
                           APP_UART_FIFO_RX_SIZE,
                           APP_UART_FIFO_TX_SIZE,
                           mt_uart_event_handle,
                           APP_IRQ_PRIORITY_LOWEST,
                           err_code);
        APP_ERROR_CHECK(err_code);
        mt_serial_flush();
        APP_ERROR_CHECK(sd_clock_hfclk_request());
        m_uart_state = true;
        NRF_LOG_INFO("uart enabled");
    }
}

void mt_serial_init(void)
{
    APP_ERROR_CHECK(app_fifo_init(&m_uart_rx_fifo, m_uart_rx_buf, sizeof(m_uart_rx_buf)));
    APP_ERROR_CHECK(app_fifo_init(&m_uart_tx_fifo, m_uart_tx_buf, sizeof(m_uart_tx_buf)));
    app_fifo_flush(&m_uart_rx_fifo);
    app_fifo_flush(&m_uart_tx_fifo);

    APP_ERROR_CHECK(
        app_timer_create(&m_rx_over_timer, APP_TIMER_MODE_REPEATED, rx_over_timeout_handler));
}