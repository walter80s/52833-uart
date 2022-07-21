/* Copyright (C) Shenzhen Minew Technologies Co., Ltd
   All rights reserved. */

#ifndef MT_LOGIC_H__
#define MT_LOGIC_H__

/**
 * @brief 定义设备开关机状态.
 */
enum {
    DEV_POWEROFF,
    DEV_POWERON,
};

/**
 * @brief 业务逻辑层的初始化.
 */
void mt_logic_init(void);

#endif