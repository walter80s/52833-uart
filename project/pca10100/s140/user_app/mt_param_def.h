
#ifndef MT_PARAM_DEF_H__
#define MT_PARAM_DEF_H__

/* 软件配置 */
#define DEF_ADV_NAME     "OneBase IR Sauna" /**< 默认设备名称 */
#define DEF_CONN_PSWD    "12345678"         /**< 默认连接密码 */
#define DEF_CONN_INT_MIN 20                 /**< 默认最小连接间隔单位 ms */
#define DEF_CONN_INT_MAN 40                 /**< 默认最大连接间隔单位 ms */

#define SLAVE_LATENCY                  0                               /**< 从机延迟 */
#define CONN_SUP_TIMEOUT               MSEC_TO_UNITS(4000, UNIT_10_MS) /**< 连接超时时间 */
#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(100) /**< 第一次连接参数更新时间 */
#define NEXT_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000) /**< 第二次连接参数更新时间 */
#define MAX_CONN_PARAMS_UPDATE_COUNT   3                     /**< 连接参数更新最大次数 */

#define DEF_ADV_INTERVAL 1 /**< 默认广播间隔单位 100ms*/
#define DEF_TX_POWER     4 /**< 默认发射功率 4dbm*/

/* 硬件配置 */
#define UART_RX_PIN  8 /**< 串口接收引脚. */
#define UART_TX_PIN  6 /**< 串口发送引脚. */
#define UART_RTS_PIN 0 /**< 流控引脚. */
#define UART_CTS_PIN 0 /**< 流控引脚. */

#endif