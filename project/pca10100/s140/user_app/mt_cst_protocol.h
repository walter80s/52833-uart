/* Copyright (C) Shenzhen Minew Technologies Co., Ltd
   All rights reserved. */

#ifndef MT_CST_PROTOCOL_H__
#define MT_CST_PROTOCOL_H__

#include <stdint.h>
#include "ble_ctcs.h"

/* clang-format off */
#define OPEN_ASE         0
#define DEF_PASSCODE      "12345678"            /* 默认密码 */

#define READ_STATU_CMD    {0x5a,0x03,0x00,0x00} /* 读取主板当前工作状态 */
#define SET_POWER_CMD     {0x5a,0x04,0x01,0x00,0x00} /* 设置主板开关机 */
#define SET_TEMP          {0x5a,0x05,0x02,0x00,0x00,0x00} /* 设置桑拿房温度 */
#define SET_TIME          {0x5a,0x05,0x03,0x00,0x00,0x00} /* 设置桑拿房时间 */
#define SET_LIGHT         {0x5a,0x04,0x00,0x00,0x00} /* 设置桑拿房 led 开关 */

#define DEF_PRODUCT_INFO   0x01 /* 默认蓝牙生产信息 */
#define DEF_HARDWARE_INFO  0x0b /* 默认蓝牙硬件信息 */
#define DEF_SOFTWARE_INFO  0x17 /* 默认蓝牙软件信息 */

#define ROOM_INFO_DATA_LEN 19  /* 串口回复 room 信息数据长度 */
/* clang-format on */

/**
 * @brief 串口指令处理状态
 */
enum {
    CMD_IDLE = 0,   /**< 无指令处理 */
    CMD_PROCESSING, /**< 指令处理中 */
};

/**
 * @brief APP 指令类型. 
 */
enum {
    READ_BLE_INFO = 0,
    READ_ROOM_INFO,
    SET_ROOM_INFO,
};

/**
 * @brief 串口指令类型. 
 */
enum {
    ROOM_CMD_READ_STATU = 0,
    ROOM_CMD_SET_POWER,
    ROOM_CMD_SET_TEMP,
    ROOM_CMD_SET_TIME,
    ROOM_CMD_SET_LIGHT1,
    ROOM_CMD_SET_LIGHT2,
    ROOM_CMD_SET_LIGHT3,
    ROOM_CMD_SET_LIGHT4,
    ROOM_CMD_SET_END,
};

/**
 * @brief 定义 room 项结构体类型
 */
typedef struct {
    unsigned char ID;
    unsigned char Byte_Count;
    unsigned char Variable;
} __attribute__((packed, aligned(1)))Feature_Def;

/**
 * @brief 定义读取指令结构体类型
 */
typedef struct {
    unsigned char  Head;
    unsigned char  Request_Type;
    unsigned short Attr_MsgId;
    unsigned char  Attr_MAC[6];
    unsigned char  Attr_User[16];
    unsigned char  Attr_RequestTime[4];
} __attribute__((packed, aligned(1)))Info_Request_Pack_Def;

/**
 * @brief 定义 app 写入结构体类型
 */
typedef struct {
    unsigned char  Head;
    unsigned char  Request_Type;
    unsigned short Attr_MsgId;
    unsigned char  Attr_MAC[6];
    unsigned char  Attr_User[16];
    unsigned char  Attr_RequestTime[4];
    Feature_Def    Attr_Power;
    Feature_Def    Attr_TargetTemp;
    Feature_Def    Attr_Timer;
    Feature_Def    Attr_Light1;
    Feature_Def    Attr_Light2;
    Feature_Def    Attr_Light3;
    Feature_Def    Attr_Light4;
} __attribute__((packed, aligned(1)))Room_Info_Set_Pack_Def;

/**
 * @brief 定义回复 app 头结构体类型
 */
typedef struct {
    unsigned char  Head;
    unsigned char  Request_Type;
    unsigned short Attr_MsgId;
    unsigned char  Attr_MAC[6];
} __attribute__((packed, aligned(1)))Response_Head_Def;

/**
 * @brief 定义回复 app ble 信息结构体类型
 */
typedef struct {
    unsigned char  Head;
    unsigned char  Request_Type;
    unsigned short Attr_MsgId;
    unsigned char  Attr_MAC[6];
    unsigned char  Attr_User[8];
    unsigned char  Attr_RequestTime[4];
    Feature_Def    Attr_Product;
    Feature_Def    Attr_Hardware;
    Feature_Def    Attr_Firmware;
} __attribute__((packed, aligned(1)))Device_Info_Response_Pack_Def;

/**
 * @brief 定义回复 app 结构体类型
 */
typedef struct {
    unsigned char  Head;
    unsigned char  Request_Type;
    unsigned short Attr_MsgId;
    unsigned char  Attr_MAC[6];
    unsigned char  Attr_User[8];
    unsigned char  Attr_RequestTime[4];
    Feature_Def    Attr_Power;
    Feature_Def    Attr_TargetTemp;
    Feature_Def    Attr_MeasuredTemp;
    Feature_Def    Attr_Timer;
    Feature_Def    Attr_Remianing;
    Feature_Def    Attr_Light1;
    Feature_Def    Attr_Light2;
    Feature_Def    Attr_Light3;
    Feature_Def    Attr_Light4;
} __attribute__((packed, aligned(1)))Room_Info_Response_Pack_Def;

/**
 * @brief 大小端转换器.
 */
#define BIG_LITTLE_ENDIAN_CONVERTER(ptr, len)            \
    do {                                                 \
        uint8_t  tmp;                                    \
        uint8_t *pointer = (ptr);                        \
        for (uint16_t i = 0; i < (len) / 2; ++i) {       \
            tmp                  = pointer[i];           \
            pointer[i]           = pointer[(len)-1 - i]; \
            pointer[(len)-1 - i] = tmp;                  \
        }                                                \
    } while (0)

/**
 * @brief 计算 crc
 *
 * @param[in] p_buf 指向存放要计算 crc 的地址.
 * @param[in] len   计算 crc 的数据长度.
 * 
 * @return[out] 输出计算好的 crc 数据.
 */
uint8_t GetCRC8(uint8_t *p_buf, uint8_t len);

/**
 * @brief 接收服务事件处理函数
 *
 * @param[in] p_evt 指向服务事件.
 */
void ctcs_data_handler(ble_ctcs_evt_t *p_evt);

/**
 * @brief 串口接收完成处理函数
 *
 * @param[in] p_data 指向接收的数据.
 * @param[in] len    接收的数据长度.
 */
void uart_receive_handler(uint8_t *p_data, uint16_t len);

/**
 * @brief 用户协议层初始化
 */
void mt_user_protocol_init(void);

#endif
