
#ifndef MT_SERIAL_H__
#define MT_SERIAL_H__

#include <stdint.h>
#include <stdbool.h>


#define SERIAL_RX_FIFO_SIZE       1024 /* 串口接收 FIFO 长度. */
#define SERIAL_TX_FIFO_SIZE       1024 /* 串口发送 FIFO 长度. */

/* clang-format off */
/**
 * @brief 串口波特率列表 
 */
#define UART_BAUDRATE_ARRAY {          \
    UARTE_BAUDRATE_BAUDRATE_Baud9600,  \
    UARTE_BAUDRATE_BAUDRATE_Baud19200, \
    UARTE_BAUDRATE_BAUDRATE_Baud38400, \
    UARTE_BAUDRATE_BAUDRATE_Baud57600, \
    UARTE_BAUDRATE_BAUDRATE_Baud115200 \
}

/**
 * @brief 默认的串口配置参数
 */
#define DEF_SERIAL_CFG {                                  \
        .rx_pin_no    = UART_RX_PIN,                      \
        .tx_pin_no    = UART_TX_PIN,                      \
        .rts_pin_no   = UART_RTS_PIN,                     \
        .cts_pin_no   = UART_CTS_PIN,                     \
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,   \
        .use_parity   = false,                            \
        .baud_rate    = UARTE_BAUDRATE_BAUDRATE_Baud38400 \
}
/* clang-format on */

/**
 * @brief 串口模块事件类型. 
 */
typedef enum {
    SERIAL_EVT_RX_OVER,
} serial_evt_id_t;

/**
 * @brief 串口模块上报事件信息结构体. 
 */
typedef struct {
    serial_evt_id_t evt_id;
    void *          p_data;
    uint32_t        len;
} serial_evt_t;

/**
 * @brief 模块回调函数类型. 
 */
typedef void (*serial_callback_t)(serial_evt_t *);

/**
 * @brief 获取串口是否开启.
 *
 * @return 开启时返回 true, 否则返回 false。
 */
bool mt_is_serial_enabled(void);

/**
 * @brief 获取串口是否发送完成.
 *
 * @return 发送完成 true, 否则返回 false。
 */
bool mt_is_serail_tx_complete(void);

/**
 * @brief 将要通过串口发送的数据放入发送 fifo, 并且开始发送.
 */
uint32_t mt_serial_put(uint8_t *p_data, uint16_t len);

/**
 * @brief 将串口模块的现场设置为空闲的状态.
 */
void mt_serial_flush(void);

/**
 * @brief 向串口模块注册回调函数.
 */
void mt_serial_hdl_register(serial_callback_t cb);

/**
 * @brief 开关串口函数.
 */
void mt_serial_ctrl_handler(bool ctrl);

/**
 * @brief 串口模块初始化.
 */
void mt_serial_init(void);

#endif