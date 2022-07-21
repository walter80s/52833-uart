
#include "sdk_common.h"
#include "ble.h"
#include "ble_ctcs.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "mt_s_public.h"

#define NRF_LOG_MODULE_NAME ble_ctcs
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

/* clang-format off */
#define CTCS_BASE_UUID {{0xfb, 0x34, 0x9b, 0x5f, \
                        0x80, 0x00, 0x00, 0x80, \
                        0x00, 0x10, 0x00, 0x00, \
                        0x00, 0x00, 0x00, 0x00}}
/* clang-format on */

/**
 * @brief ctcs 连接事件处理.
 *
 * @param[in] p_ctcs   
 * @param[in] p_ble_evt 事件
 */
static void on_connect(ble_ctcs_t *p_ctcs, ble_evt_t const *p_ble_evt)
{
    p_ctcs->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

/**
 * @brief ctcs 断开连接事件处理.
 *
 * @param[in] p_ctcs   
 * @param[in] p_ble_evt 事件
 */
static void on_disconnect(ble_ctcs_t *p_ctcs, ble_evt_t const *p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_ctcs->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/**
 * @brief ctcs 写入事件处理.
 *
 * @param[in] p_ctcs   
 * @param[in] p_ble_evt 事件
 */
static void on_write(ble_ctcs_t *p_ctcs, ble_evt_t const *p_ble_evt)
{
    ble_gatts_evt_write_t const *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    ble_ctcs_evt_t               evt;
    memset(&evt, 0, sizeof(ble_ctcs_evt_t));
    evt.p_ctcs = p_ctcs;
    if (p_ctcs->data_handler != NULL) {
        evt.params.attachment_data.p_data = p_evt_write->data;
        evt.params.attachment_data.length = p_evt_write->len;
        evt.uuid                          = p_evt_write->uuid.uuid;
        if (p_evt_write->handle == p_ctcs->passcode_handles.value_handle) {
            evt.type = BLE_CTCS_EVT_PASSCODE_RX_DATA;
            p_ctcs->data_handler(&evt);
        } else if (p_evt_write->handle == p_ctcs->transport_handles.value_handle) {
            evt.type = BLE_CTCS_EVT_TRANSPORT_RX_DATA;
            p_ctcs->data_handler(&evt);
        }
    }
}

void ble_ctcs_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL)) {
        return;
    }

    ble_ctcs_t *p_ctcs = (ble_ctcs_t *)p_context;

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_ctcs, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_ctcs, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_ctcs, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE: {
            //notify with empty data that some tx was completed.
            ble_ctcs_evt_t evt = { .type = BLE_CTCS_EVT_TX_COMPLETE, .p_ctcs = p_ctcs };
            p_ctcs->data_handler(&evt);
            break;
        }
        default:
            // No implementation needed.
            break;
    }
}

uint32_t ble_ctcs_init(ble_ctcs_t *p_ctcs, ble_ctcs_init_t const *p_ctcs_init)
{
    ble_uuid_t            ble_uuid;
    ble_gatt_char_props_t char_props;
    ble_gatts_attr_md_t   m_attr_md;
    ble_uuid128_t         ctcs_base_uuid = CTCS_BASE_UUID;

    VERIFY_PARAM_NOT_NULL(p_ctcs);
    VERIFY_PARAM_NOT_NULL(p_ctcs_init);
    p_ctcs->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_ctcs->data_handler            = p_ctcs_init->data_handler;
    p_ctcs->is_notification_enabled = false;

    VERIFY_SUCCESS(sd_ble_uuid_vs_add(&ctcs_base_uuid, &p_ctcs->uuid_type));
    ble_uuid.type = p_ctcs->uuid_type;
    ble_uuid.uuid = CTCS_SERVER_UUID;
    VERIFY_SUCCESS(
        sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_ctcs->service_handle));

    /* 添加 Attachment 特征值 */
    fill_in_character_paraments(&char_props, &m_attr_md);
    char_props.read = 0;
    VERIFY_SUCCESS(common_char_add(p_ctcs->uuid_type,
                                   p_ctcs->service_handle,
                                   char_props,
                                   &p_ctcs->passcode_handles,
                                   m_attr_md,
                                   CTCS_PASSCODE_UUID));
    /* 添加 Zone Boost 特征值 */
    fill_in_character_paraments(&char_props, &m_attr_md);
    char_props.read = 0;
    VERIFY_SUCCESS(common_char_add(p_ctcs->uuid_type,
                                   p_ctcs->service_handle,
                                   char_props,
                                   &p_ctcs->transport_handles,
                                   m_attr_md,
                                   CTCS_TRANSPORT_UUID));

    return NRF_SUCCESS;
}

uint32_t
ble_ctcs_notify_send(ble_ctcs_t *p_ctcs, uint8_t *p_string, uint16_t *p_length, uint16_t uuid)
{
    ble_gatts_hvx_params_t hvx_params;

    VERIFY_PARAM_NOT_NULL(p_ctcs);

    if (p_ctcs->conn_handle == BLE_CONN_HANDLE_INVALID) {
        return NRF_ERROR_INVALID_STATE;
    }

    if (*p_length > BLE_CTCS_MAX_DATA_LEN) {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));
    switch (uuid) {
        case CTCS_PASSCODE_UUID:
            hvx_params.handle = p_ctcs->passcode_handles.value_handle;
            break;

        case CTCS_TRANSPORT_UUID:
            hvx_params.handle = p_ctcs->transport_handles.value_handle;
            break;

        default:
            return NRF_ERROR_INVALID_PARAM;
            break;
    }

    hvx_params.p_data = p_string;
    hvx_params.p_len  = p_length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_ctcs->conn_handle, &hvx_params);
}
