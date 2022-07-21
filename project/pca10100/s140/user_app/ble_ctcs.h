
#ifndef BLE_CTCS_H__
#define BLE_CTCS_H__

#include <stdint.h>
#include <stdbool.h>
#include "sdk_config.h"
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"

/* clang-format off */
#define BLE_CTCS_DEF(_name)                                                                          \
static ble_ctcs_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                  \
                     2,                                                                              \
                     ble_ctcs_on_ble_evt, &_name)

#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2

#define BLE_USERS_MAX_RX_LEN        BLE_CTCS_MAX_DATA_LEN 

/**@brief   Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
    #define BLE_CTCS_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
#else
    #define BLE_CTCS_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
    #warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif

#define CTCS_SERVER_UUID                    0xff00                    /* 服务 UUID */
#define CTCS_PASSCODE_UUID                  0xff01                    /* 密码特征 UUID */
#define CTCS_TRANSPORT_UUID                 0xff02                    /* 透传特征 UUID */
/* clang-format on */

/**< ctcs 服务事件 */
typedef enum {
    BLE_CTCS_EVT_PASSCODE_RX_DATA,
    BLE_CTCS_EVT_TRANSPORT_RX_DATA,
    BLE_CTCS_EVT_TX_COMPLETE,
    BLE_CTCS_EVT_COMM_STARTED,
    BLE_CTCS_EVT_COMM_STOPPED,    
} ble_ctcs_evt_type_t;

typedef struct ble_ctcs_s ble_ctcs_t;

/**< ctcs 接收到数据结构体 */
typedef struct {
    uint8_t const *p_data; /**< 接收到的数据. */
    uint16_t       length; /**< 接收到的数据长度. */
} ble_ctcs_evt_rx_data_t;

/**< ctcs 服务事件结构体 */
typedef struct {
    ble_ctcs_evt_type_t type;
    ble_ctcs_t *        p_ctcs;
    uint16_t            uuid;
    union {
        ble_ctcs_evt_rx_data_t attachment_data;
    } params;
} ble_ctcs_evt_t;

/**< ctcs 服务回调函数指针 */
typedef void (*ble_ctcs_data_handler_t)(ble_ctcs_evt_t *p_evt);

/**< ctcs 服务初始化结构体 */
typedef struct {
    ble_ctcs_data_handler_t data_handler;
} ble_ctcs_init_t;

/**< ctcs 服务句柄结构体 */
struct ble_ctcs_s {
    uint8_t                  uuid_type;
    uint16_t                 service_handle;
    ble_gatts_char_handles_t passcode_handles;
    ble_gatts_char_handles_t transport_handles;

    uint16_t                conn_handle;
    bool                    is_notification_enabled;
    ble_ctcs_data_handler_t data_handler;
};

/**
 * @brief 初始化 ctcs 服务.
 *
 * @param[out] p_ctcs      用户服务结构体
 * @param[in]  p_ctcs_init 初始化服务所需要的信息
 * 
 * @retval  NRF_SUCCESS 成功 错误返回错误码
 */
uint32_t ble_ctcs_init(ble_ctcs_t *p_ctcs, ble_ctcs_init_t const *p_ctcs_init);

/**
 * @brief ctcs 服务 ble 事件处理函数.
 *
 * @param[in] p_ble_evt   事件
 * @param[in] p_ctcs_init 未使用
 */
void ble_ctcs_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context);

/**
 * @brief ctcs 服务发送 notify 函数.
 *
 * @param[in]  p_ctcs      用户服务结构体
 * @param[in]  p_string    要发送的 buff 地址
 * @param[in]  p_length    要发送的数据长度地址
 * @param[in]  uuid        要发送 notify 的特征值 uuid
 * 
 * @retval  NRF_SUCCESS 成功 错误返回错误码
 */
uint32_t
ble_ctcs_notify_send(ble_ctcs_t *p_ctcs, uint8_t *p_string, uint16_t *p_length, uint16_t uuid);


#endif
