
#include "mt_ble_service.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "app_error.h"
#include "ble_ctcs.h"
#include "mt_cst_protocol.h"
#include "ble_dfu.h"

#define NRF_LOG_MODULE_NAME ble_srv
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

NRF_BLE_QWR_DEF(m_qwr);
BLE_CTCS_DEF(m_ctcs);

static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
    switch (event) {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE: {
            NRF_LOG_INFO("Device is preparing to enter bootloader mode.");
            break;
        }

        case BLE_DFU_EVT_BOOTLOADER_ENTER:
            NRF_LOG_INFO("Device will enter bootloader mode.");
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
            NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
            NRF_LOG_ERROR("Request to send a response to client failed.");
            APP_ERROR_CHECK(false);
            break;

        default:
            NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
            break;
    }
}

/**
 * @brief 处理 Queue_Write 模块的错误
 *
 * @param[in] nrf_error 模块出现的具体的错误码
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

uint32_t ble_ctcs_s_send_handler(uint8_t *p_data, uint16_t len, uint16_t uuid)
{
    return ble_ctcs_notify_send(&m_ctcs, p_data, &len, uuid);
}

/**
 * @brief ctcs 服務初始化.
 */
static void user_ctcs_init(void)
{
    ble_ctcs_init_t           ctcs_init;
    nrf_ble_qwr_init_t        qwr_init  = { 0 };
    ble_dfu_buttonless_init_t dfus_init = { 0 };
    qwr_init.error_handler              = nrf_qwr_error_handler;
    APP_ERROR_CHECK(nrf_ble_qwr_init(&m_qwr, &qwr_init));

    memset(&ctcs_init, 0, sizeof(ctcs_init));
    ctcs_init.data_handler = ctcs_data_handler;
    APP_ERROR_CHECK(ble_ctcs_init(&m_ctcs, &ctcs_init));

    dfus_init.evt_handler = ble_dfu_evt_handler;
    APP_ERROR_CHECK(ble_dfu_buttonless_init(&dfus_init));
}

void ble_service_init(void)
{
    user_ctcs_init();
}
