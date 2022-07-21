
#include "mt_cst_protocol.h"
#include "mt_ble_service.h"
#include "mt_serial.h"
#include "app_timer.h"
#include "mt_ble_service.h"
#include "mt_ble_cfg.h"
#include "ble_gap.h"
#include "aes.h"

#define NRF_LOG_MODULE_NAME cst_p
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define CMD_TIMEOUT_TIME APP_TIMER_TICKS(1000) /**< 命令超时定时器超时时间 */

static bool          m_cmd_statu       = CMD_IDLE; /* 指示串口指令处理状态 */
static uint8_t       m_disconnect_flag = 1;        /* 指示是否要断开连接 0 断开连接 */
static uint32_t      m_sys_clock       = 0;        /**< 记录系统时间. */
static uint8_t       m_device_mac[6]   = { 0 };    /**< 记录蓝牙设备 mac . */
static uint8_t       m_device_id[8]    = { 0 };    /**< 记录蓝牙设备 id . */
static const uint8_t AESKey[]          = DEF_PASSCODE; /**< aes key . */
static uint8_t       m_tmp_buf[128]    = { 0 };        /**< 存放解密后的 app 数据 . */
static uint8_t processed_itm = ROOM_CMD_SET_POWER;     /**< 指示处理 app 设置 room 项目 . */
static bool          m_set_for_statu_response = false; /**< 指示 app 发送设置指令而读取桑拿房状态 true . */

APP_TIMER_DEF(m_timeout_timer); /**< 命令超时定时器. */
APP_TIMER_DEF(m_sys_timer);     /**< 系统时间定时器. */

const uint8_t CRC_Table[256] = {
    0,   49,  98,  83,  196, 245, 166, 151, 185, 136, 219, 234, 125, 76,  31,  46,  67,  114, 33,
    16,  135, 182, 229, 212, 250, 203, 152, 169, 62,  15,  92,  109, 134, 183, 228, 213, 66,  115,
    32,  17,  63,  14,  93,  108, 251, 202, 153, 168, 197, 244, 167, 150, 1,   48,  99,  82,  124,
    77,  30,  47,  184, 137, 218, 235, 61,  12,  95,  110, 249, 200, 155, 170, 132, 181, 230, 215,
    64,  113, 34,  19,  126, 79,  28,  45,  186, 139, 216, 233, 199, 246, 165, 148, 3,   50,  97,
    80,  187, 138, 217, 232, 127, 78,  29,  44,  2,   51,  96,  81,  198, 247, 164, 149, 248, 201,
    154, 171, 60,  13,  94,  111, 65,  112, 35,  18,  133, 180, 231, 214, 122, 75,  24,  41,  190,
    143, 220, 237, 195, 242, 161, 144, 7,   54,  101, 84,  57,  8,   91,  106, 253, 204, 159, 174,
    128, 177, 226, 211, 68,  117, 38,  23,  252, 205, 158, 175, 56,  9,   90,  107, 69,  116, 39,
    22,  129, 176, 227, 210, 191, 142, 221, 236, 123, 74,  25,  40,  6,   55,  100, 85,  194, 243,
    160, 145, 71,  118, 37,  20,  131, 178, 225, 208, 254, 207, 156, 173, 58,  11,  88,  105, 4,
    53,  102, 87,  192, 241, 162, 147, 189, 140, 223, 238, 121, 72,  27,  42,  193, 240, 163, 146,
    5,   52,  103, 86,  120, 73,  26,  43,  188, 141, 222, 239, 130, 179, 224, 209, 70,  119, 36,
    21,  59,  10,  89,  104, 255, 206, 157, 172
};

uint8_t GetCRC8(uint8_t *p_buf, uint8_t len)
{
    uint8_t  crc = 0;
    uint32_t i;
    for (i = 0; i < len - 1; i++) { crc = CRC_Table[p_buf[i] ^ crc]; }
    return crc;
}

/**
 * @brief 命令超时定时器超时处理函数.
 */
static void tm_timeout_handler(void *p_context)
{
    m_cmd_statu = CMD_IDLE;
}

/**
 * @brief 系统定时器超时处理函数.
 */
static void sys_timeout_handler(void *p_context)
{
    m_sys_clock++;
}

static void check_passcode(uint8_t *p_data, uint16_t len)
{
    uint8_t  passcode[] = DEF_PASSCODE;
    uint16_t tmp        = 0;
    tmp                 = sizeof(passcode) - 1;
    if ((tmp != len) || memcmp(passcode, p_data, tmp)) {
        m_disconnect_flag = 0;
    } else {
        m_disconnect_flag = 1;
    }
    ble_ctcs_s_send_handler(&m_disconnect_flag, 1, CTCS_PASSCODE_UUID);
}

/**
 * @brief 获取桑拿房状态
 */
static void get_room_statu(void)
{
    if (m_cmd_statu) {
        return;
    }
    uint8_t cmd[] = READ_STATU_CMD;
    uint8_t len   = sizeof(cmd);
    cmd[len - 1]  = GetCRC8(cmd, len);
    mt_serial_put(cmd, len);
    m_cmd_statu = CMD_PROCESSING;
    app_timer_stop(m_timeout_timer);
    APP_ERROR_CHECK(app_timer_start(m_timeout_timer, CMD_TIMEOUT_TIME, NULL));
}

/**
 * @brief 设置桑拿房开关机
 * 
 * @param[in] statu 0x00 关机 0x01 开机
 */
static void set_room_power(uint8_t statu)
{
    if (m_cmd_statu) {
        return;
    }
    uint8_t cmd[] = SET_POWER_CMD;
    uint8_t len   = sizeof(cmd);
    cmd[len - 2]  = statu;
    cmd[len - 1]  = GetCRC8(cmd, len);
    mt_serial_put(cmd, len);
    m_cmd_statu = CMD_PROCESSING;
    app_timer_stop(m_timeout_timer);
    APP_ERROR_CHECK(app_timer_start(m_timeout_timer, CMD_TIMEOUT_TIME, NULL));
}

/**
 * @brief 设置桑拿房温度
 * 
 * @param[in] temp app 设置的温度
 */
static void set_room_temp(uint16_t temp)
{
    if (m_cmd_statu) {
        return;
    }
    uint8_t cmd[] = SET_TEMP;
    uint8_t len   = sizeof(cmd);
    cmd[len - 3]  = (temp >> 8) & 0xFF;
    cmd[len - 2]  = temp & 0xFF;
    cmd[len - 1]  = GetCRC8(cmd, len);
    mt_serial_put(cmd, len);
    m_cmd_statu = CMD_PROCESSING;
    app_timer_stop(m_timeout_timer);
    APP_ERROR_CHECK(app_timer_start(m_timeout_timer, CMD_TIMEOUT_TIME, NULL));
}

/**
 * @brief 设置桑拿房时间
 * 
 * @param[in] time  app 设置的时间
 */
static void set_room_time(uint16_t time)
{
    if (m_cmd_statu) {
        return;
    }
    uint8_t cmd[] = SET_TIME;
    uint8_t len   = sizeof(cmd);
    cmd[len - 3]  = (time >> 8) & 0xFF;
    cmd[len - 2]  = time & 0xFF;
    cmd[len - 1]  = GetCRC8(cmd, len);
    mt_serial_put(cmd, len);
    m_cmd_statu = CMD_PROCESSING;
    app_timer_stop(m_timeout_timer);
    APP_ERROR_CHECK(app_timer_start(m_timeout_timer, CMD_TIMEOUT_TIME, NULL));
}

/**
 * @brief 设置桑拿房灯状态
 * 
 * @param[in] statu    led 状态
 * @param[in] led_num  led 编号
 */
static void room_light_control(uint8_t statu, uint8_t led_num)
{
    if (m_cmd_statu) {
        return;
    }
    uint8_t cmd[] = SET_LIGHT;
    uint8_t len   = sizeof(cmd);
    cmd[len - 3]  = led_num + 3; /* 操作对应 led 命令编号 */
    cmd[len - 2]  = statu;
    cmd[len - 1]  = GetCRC8(cmd, len);
    mt_serial_put(cmd, len);
    m_cmd_statu = CMD_PROCESSING;
    app_timer_stop(m_timeout_timer);
    APP_ERROR_CHECK(app_timer_start(m_timeout_timer, CMD_TIMEOUT_TIME, NULL));
}

/**
 * @brief 处理 app 设置 room 参数
 */
static void app_set_data_process(void)
{
    Room_Info_Set_Pack_Def *p_data   = (Room_Info_Set_Pack_Def *)m_tmp_buf;
    uint16_t                tmp_data = 0;
    NRF_LOG_INFO("processed_itm=%d len=%d", processed_itm,sizeof(Room_Info_Set_Pack_Def));
    switch (processed_itm) {
        case ROOM_CMD_SET_POWER:
            processed_itm = ROOM_CMD_SET_TEMP;
            if (p_data->Attr_Power.ID) {
                set_room_power(p_data->Attr_Power.Variable);
            } else {
                app_set_data_process();
            }
            break;
        case ROOM_CMD_SET_TEMP:
            processed_itm = ROOM_CMD_SET_TIME;
            if (p_data->Attr_TargetTemp.ID) {
                tmp_data |= p_data->Attr_TargetTemp.Variable;
                set_room_temp(tmp_data);
            } else {
                app_set_data_process();
            }

            break;
        case ROOM_CMD_SET_TIME:
            processed_itm = ROOM_CMD_SET_LIGHT1;
            if (p_data->Attr_Timer.ID) {
                tmp_data |= p_data->Attr_Timer.Variable;
                set_room_time(tmp_data);
            } else {
                app_set_data_process();
            }

            break;
        case ROOM_CMD_SET_LIGHT1:
            processed_itm = ROOM_CMD_SET_LIGHT2;
            if (p_data->Attr_Light1.ID) {
                room_light_control(p_data->Attr_Light1.Variable, 1);
            } else {
                app_set_data_process();
            }

            break;
        case ROOM_CMD_SET_LIGHT2:
            processed_itm = ROOM_CMD_SET_LIGHT3;
            if (p_data->Attr_Light2.ID) {
                room_light_control(p_data->Attr_Light2.Variable, 2);
            } else {
                app_set_data_process();
            }

            break;
        case ROOM_CMD_SET_LIGHT3:
            processed_itm = ROOM_CMD_SET_LIGHT4;
            if (p_data->Attr_Light3.ID) {
                room_light_control(p_data->Attr_Light3.Variable, 3);
            } else {
                app_set_data_process();
            }

            break;
        case ROOM_CMD_SET_LIGHT4:
            processed_itm = ROOM_CMD_SET_END;
            if (p_data->Attr_Light4.ID) {
                room_light_control(p_data->Attr_Light4.Variable, 4);
            } else {
                app_set_data_process();
            }
            break;

        case ROOM_CMD_SET_END:
            get_room_statu();
            m_set_for_statu_response = true;
            break;

        default:
            break;
    }
}

/**
 * @brief 回复 app 读取 room 信息命令
 * 
 * @param[in] p_data  指向串口回复数据
 * @param[in] len     串口回复数据长度
 */
static void response_app_room_statu(uint8_t *p_data, uint16_t len)
{
    if (((len - 1) != ROOM_INFO_DATA_LEN) || (p_data[1] != ROOM_INFO_DATA_LEN)) {
        NRF_LOG_INFO("room info length error=%d", len);
        return;
    }
    Room_Info_Response_Pack_Def response;

    uint8_t tmp_aes_buf[sizeof(Room_Info_Response_Pack_Def) + 16]  = { 0 };
    uint8_t tmp_send_buf[sizeof(Room_Info_Response_Pack_Def) + 16] = { 0 };
    uint8_t tmp_len = sizeof(Room_Info_Response_Pack_Def);

    memcpy(&response, (uint8_t *)m_tmp_buf, sizeof(Response_Head_Def));

    response.Head = 0xa5;
    memcpy(response.Attr_MAC, m_device_mac, 6);

    response.Attr_RequestTime[3] = m_sys_clock;
    response.Attr_RequestTime[2] = m_sys_clock >> 8;
    response.Attr_RequestTime[1] = m_sys_clock >> 16;
    response.Attr_RequestTime[0] = m_sys_clock >> 24;
    memcpy(response.Attr_User, m_device_id, 8);
    response.Attr_Power.ID         = 0x04;
    response.Attr_Power.Byte_Count = 0x01;
    response.Attr_Power.Variable   = p_data[3];

    response.Attr_TargetTemp.ID         = 0x05;
    response.Attr_TargetTemp.Byte_Count = 0x01;
    response.Attr_TargetTemp.Variable   = p_data[5];

    response.Attr_MeasuredTemp.ID         = 0x06;
    response.Attr_MeasuredTemp.Byte_Count = 0x01;
    response.Attr_MeasuredTemp.Variable   = p_data[7];

    response.Attr_Timer.ID         = 0x07;
    response.Attr_Timer.Byte_Count = 0x01;
    response.Attr_Timer.Variable   = p_data[9];

    response.Attr_Remianing.ID         = 0x08;
    response.Attr_Remianing.Byte_Count = 0x01;
    response.Attr_Remianing.Variable   = p_data[11];

    response.Attr_Light1.ID         = 0x09;
    response.Attr_Light1.Byte_Count = 0x01;
    response.Attr_Light1.Variable   = p_data[12];

    response.Attr_Light2.ID         = 0x0a;
    response.Attr_Light2.Byte_Count = 0x01;
    response.Attr_Light2.Variable   = p_data[13];

    response.Attr_Light3.ID         = 0x0b;
    response.Attr_Light3.Byte_Count = 0x01;
    response.Attr_Light3.Variable   = p_data[14];

    response.Attr_Light4.ID         = 0x0c;
    response.Attr_Light4.Byte_Count = 0x01;
    response.Attr_Light4.Variable   = p_data[15];

    memcpy(&tmp_aes_buf, (uint8_t *)&response, tmp_len);
#if OPEN_ASE
    for (uint8_t i = 0; i < tmp_len; i += 16) {
        AES128_ECB_encrypt(&tmp_aes_buf[i], AESKey, &tmp_send_buf[i]);
    }
#else
    memcpy(tmp_send_buf, (uint8_t *)&response, tmp_len);
#endif
    if(m_set_for_statu_response){
        m_set_for_statu_response = false;
        response.Request_Type = 0x02;
    }
    ble_ctcs_s_send_handler(tmp_send_buf, tmp_len, CTCS_TRANSPORT_UUID);
}

/**
 * @brief 回复 app 读取 ble 设备信息指令
 */
static void ble_info_response(void)
{
    Device_Info_Response_Pack_Def response;

    uint8_t tmp_aes_buf[sizeof(Device_Info_Response_Pack_Def) + 16]  = { 0 };
    uint8_t tmp_send_buf[sizeof(Device_Info_Response_Pack_Def) + 16] = { 0 };
    uint8_t tmp_len = sizeof(Device_Info_Response_Pack_Def);

    memcpy(&response, (uint8_t *)m_tmp_buf, sizeof(Response_Head_Def));

    response.Head = 0xa5;
    memcpy(response.Attr_MAC, m_device_mac, 6);

    response.Attr_RequestTime[3] = m_sys_clock;
    response.Attr_RequestTime[2] = m_sys_clock >> 8;
    response.Attr_RequestTime[1] = m_sys_clock >> 16;
    response.Attr_RequestTime[0] = m_sys_clock >> 24;
    memcpy(response.Attr_User, m_device_id, 8);
    response.Attr_Product.ID          = 0x01;
    response.Attr_Product.Byte_Count  = 0x01;
    response.Attr_Product.Variable    = DEF_PRODUCT_INFO;
    response.Attr_Hardware.ID         = 0x02;
    response.Attr_Hardware.Byte_Count = 0x01;
    response.Attr_Hardware.Variable   = DEF_HARDWARE_INFO;
    response.Attr_Firmware.ID         = 0x03;
    response.Attr_Firmware.Byte_Count = 0x01;
    response.Attr_Firmware.Variable   = DEF_SOFTWARE_INFO;

    memcpy(&tmp_aes_buf, (uint8_t *)&response, tmp_len);
#if OPEN_ASE
    for (uint8_t i = 0; i < tmp_len; i += 16) {
        AES128_ECB_encrypt(&tmp_aes_buf[i], AESKey, &tmp_send_buf[i]);
    }
#else
    memcpy(tmp_send_buf, (uint8_t *)&response, tmp_len);
#endif
    NRF_LOG_HEXDUMP_INFO(tmp_send_buf, tmp_len);
    ble_ctcs_s_send_handler(tmp_send_buf, tmp_len, CTCS_TRANSPORT_UUID);
}

/**
 * @brief 解密 app 发送数据
 * 
 * @param[in] p_data  指向 app 发送数据
 * @param[in] len     app 发送数据长度
 */
static void decrypt_app_data(uint8_t *p_data, uint16_t len)
{
    uint8_t tmp_buf[128] = { 0 };
    memcpy(tmp_buf, p_data, len);
    memset(m_tmp_buf, 0, 128);
    for (uint8_t i = 0; i < len; i += 16) {
        AES128_ECB_decrypt(&tmp_buf[i], AESKey, &m_tmp_buf[i]);
    }
}

void ctcs_data_handler(ble_ctcs_evt_t *p_evt)
{
    if ((p_evt->type == BLE_CTCS_EVT_TX_COMPLETE) && (m_disconnect_flag == 0)) {
        m_disconnect_flag = 1;
        mt_ble_force_disconnect();
    }
    switch (p_evt->type) {
        case BLE_CTCS_EVT_PASSCODE_RX_DATA:
            check_passcode((uint8_t *)p_evt->params.attachment_data.p_data,
                           p_evt->params.attachment_data.length);
            break;
        case BLE_CTCS_EVT_TRANSPORT_RX_DATA:
#if OPEN_ASE
            decrypt_app_data((uint8_t *)p_evt->params.attachment_data.p_data,
                             p_evt->params.attachment_data.length);
#else
            memcpy(m_tmp_buf,
                   (uint8_t *)p_evt->params.attachment_data.p_data,
                   p_evt->params.attachment_data.length);
#endif
            NRF_LOG_HEXDUMP_INFO(m_tmp_buf, p_evt->params.attachment_data.length);
            Info_Request_Pack_Def *info_pack_t = (Info_Request_Pack_Def *)m_tmp_buf;
            m_sys_clock = (uint32_t)((info_pack_t->Attr_RequestTime[0] << 24) |
                                     (info_pack_t->Attr_RequestTime[1] << 16) |
                                     (info_pack_t->Attr_RequestTime[2] << 8) |
                                     info_pack_t->Attr_RequestTime[3]);
            NRF_LOG_INFO("info_pack_t->Request_Type=%d", info_pack_t->Request_Type);

            if (info_pack_t->Head != 0x5a) {
                return;
            }
            switch (info_pack_t->Request_Type) {
                case READ_BLE_INFO:
                    ble_info_response();
                    break;
                case READ_ROOM_INFO:
                    get_room_statu();
                    break;
                case SET_ROOM_INFO:
                    processed_itm = ROOM_CMD_SET_POWER;
                    app_set_data_process();
                    break;

                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void uart_receive_handler(uint8_t *p_data, uint16_t len)
{
    ASSERT(p_data != NULL);
    if (GetCRC8(p_data, len) != p_data[len - 1]) {
        m_cmd_statu = CMD_IDLE;
        return;
    }
    switch (p_data[2]) {
        case ROOM_CMD_READ_STATU:
            response_app_room_statu(p_data, len);
            break;
        case ROOM_CMD_SET_POWER:
        case ROOM_CMD_SET_TEMP:
        case ROOM_CMD_SET_TIME:
        case ROOM_CMD_SET_LIGHT1:
        case ROOM_CMD_SET_LIGHT2:
        case ROOM_CMD_SET_LIGHT3:
        case ROOM_CMD_SET_LIGHT4:
            app_set_data_process();
            break;
        default:
            break;
    }
}

void mt_user_protocol_init(void)
{
    ble_gap_addr_t ble_gap_mac;

    m_device_id[0] = NRF_FICR->DEVICEID[0] >> 24;
    m_device_id[1] = NRF_FICR->DEVICEID[0] >> 16;
    m_device_id[2] = NRF_FICR->DEVICEID[0] >> 8;
    m_device_id[3] = NRF_FICR->DEVICEID[0];
    m_device_id[4] = NRF_FICR->DEVICEID[1] >> 24;
    m_device_id[5] = NRF_FICR->DEVICEID[1] >> 16;
    m_device_id[6] = NRF_FICR->DEVICEID[1] >> 8;
    m_device_id[7] = NRF_FICR->DEVICEID[1];

    APP_ERROR_CHECK(sd_ble_gap_addr_get(&ble_gap_mac));
    BIG_LITTLE_ENDIAN_CONVERTER(ble_gap_mac.addr, 6);
    memcpy(m_device_mac, ble_gap_mac.addr, 6);
    APP_ERROR_CHECK(
        app_timer_create(&m_timeout_timer, APP_TIMER_MODE_SINGLE_SHOT, tm_timeout_handler));
    APP_ERROR_CHECK(app_timer_create(&m_sys_timer, APP_TIMER_MODE_REPEATED, sys_timeout_handler));
    APP_ERROR_CHECK(app_timer_start(m_sys_timer, CMD_TIMEOUT_TIME, NULL));
}