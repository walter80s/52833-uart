/* Copyright (C) Shenzhen Minew Technologies Co., Ltd
   All rights reserved. */

#ifndef MT_S_PUBLIC_H__
#define MT_S_PUBLIC_H__

#include <stdint.h>
#include <stdbool.h>
#include "sdk_config.h"
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"

#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2

#define FIRST_CTCS_UUID                    0     /**< 是否使用第一组特征值 uuid. */
#define OTA_AFFECT_FLASH                   0     /**< 只做升级相对上一个版本存储改变,设置为 1 要做对应的处理. */

#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
#    define BLE_PUBLIC_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
#else
#    define BLE_PUBLIC_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
#    warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif

/**
 * @brief 添加特征值.
 *
 * @param[in]  uuid_type        uuid 类型
 * @param[in]  service_handle   服务句柄
 * @param[in]  char_props       特征值属性
 * @param[in]  char_handle      特征值句柄     
 * @param[in]  m_attr_md        读写权限 
 * @param[in]  uuid             特征值 uuid
 * 
 * @retval  NRF_SUCCESS 成功 错误返回错误码
 */
uint32_t common_char_add(uint8_t                   uuid_type,
                         uint16_t                  service_handle,
                         ble_gatt_char_props_t     char_props,
                         ble_gatts_char_handles_t *char_handle,
                         ble_gatts_attr_md_t       m_attr_md,
                         uint16_t                  uuid);

/**
 * @brief ctcs 特征值属性初始化函数.
 *
 * @param[in] p_char_props  特征值属性
 * @param[in] p_m_attr_md   读写特征值是否需要申请设置
 */
void fill_in_character_paraments(ble_gatt_char_props_t *p_char_props,
                                 ble_gatts_attr_md_t *  p_m_attr_md);

#endif