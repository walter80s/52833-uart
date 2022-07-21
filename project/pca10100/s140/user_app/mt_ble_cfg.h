/* Copyright (C) Shenzhen Minew Technologies Co., Ltd
   All rights reserved. */

#ifndef MT_BLE_CFG_H__
#define MT_BLE_CFG_H__

#include <stdint.h>
#include <stdbool.h>

#include "nrf_ble_gatt.h"

/**
 * @brief 注册 GATT 事件处理回调函数
 *
 * @param[in] cb 注册的回调函数.
 */
void mt_ble_gatt_register(nrf_ble_gatt_evt_handler_t cb);

/**
 * @brief 更新连接参数.
 */
void mt_ble_conn_param_update(void);

/**
 * @brief 获取当前是否处于连接状态.
 *
 * @return 处于连接状态返回 true, 否则返回 false.
 */
bool mt_is_ble_connected(void);

/**
 * @brief 断开当前连接.
 */
void mt_ble_force_disconnect(void);

/**
 * @brief BLE 相关的初始化
 */
void mt_ble_config_init(void);

#endif
