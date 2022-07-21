/* Copyright (C) Shenzhen Minew Technologies Co., Ltd
   All rights reserved. */

#ifndef MT_BLE_SERVICE_H__
#define MT_BLE_SERVICE_H__

#include <stdint.h>

/* clang-format off */
#define PNP_ID_VENDOR_ID_SOURCE          0x02
#define PNP_ID_VENDOR_ID                 0x05AC
#define PNP_ID_PRODUCT_ID                0x061B
#define PNP_ID_PRODUCT_VERSION           0x0001

#define MANUFACTURER_ID                  0x55AA55AA55
#define ORG_UNIQUE_ID                    0xEEBBEE

#define BLE_DIS_CERT_LIST               { 0x00, 0x01, 0x02, 0x03 }

#define USER_DEFINED_MANUFACT_NAME      "Ryder"   
#define USER_DEFINED_MODEL_NUM          "REJ6"
#define USER_HW_VERSION                 "1.0"
#define USER_FW_VERSION                 "1.2.7"
/* clang-format on */

/**
 * @brief 更新 error 通道值.
 */
void mt_update_err_char_value(void);

/**
 * @brief 通过 ctcs_server 将数据 notify 出去
 *
 * @param[in] p_data 指向将要 notify 出去的数据的首地址.
 * @param[in] len 表示数据的长度.
 * @param[in] uuid 要发送给的数据通道
 * 
 * @retval NRF_SUCCESS 成功 错误返回错误码
 */
uint32_t ble_ctcs_s_send_handler(uint8_t *p_data, uint16_t len, uint16_t uuid);

/**
 * @brief 通过 ctcs_server 特征值 read 属性更新
 *
 * @param[in] uuid     要更新的特征值通道
 * @param[in] p_data   要更新的数据.
 * @param[in] len      要更新的长度.
 * 
 * @retval NRF_SUCCESS 成功 错误返回错误码
 */
uint32_t ble_ctcs_read_update_handler(uint16_t uuid, uint8_t *p_data, uint8_t len);

/**
 * @brief 通过 mfus_server 将数据 notify 出去
 *
 * @param[in] p_data 指向将要 notify 出去的数据的首地址.
 * @param[in] len 表示数据的长度.
 * @param[in] uuid 要发送给的数据通道
 * 
 * @retval NRF_SUCCESS 成功 错误返回错误码
 */
uint32_t ble_mfus_s_send_handler(uint8_t *p_data, uint16_t len, uint16_t uuid);

/**
 * @brief 通过 otas_server 将数据 notify 出去
 *
 * @param[in] p_data 指向将要 notify 出去的数据的首地址.
 * @param[in] len 表示数据的长度.
 * @param[in] uuid 要发送给的数据通道
 * 
 * @retval NRF_SUCCESS 成功 错误返回错误码
 */
uint32_t ble_otas_s_send_handler(uint8_t *p_data, uint16_t len, uint16_t uuid);

/**
 * @brief 更新电池电量服务.
 *
 * @param[in] battery_leavel 要更新的电池电量百分比.
 */
void battery_level_update(uint8_t battery_leavel);

/**
 * @brief service 模块初始化.
 */
void ble_service_init(void);

#endif
