/* Copyright (C) Shenzhen Minew Technologies Co., Ltd
   All rights reserved. */

#include "mt_s_public.h"
#include "ble_gatt.h"
#include "ble_gatts.h"

#define NRF_LOG_MODULE_NAME s_public
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

uint32_t common_char_add(uint8_t                   uuid_type,
                         uint16_t                  service_handle,
                         ble_gatt_char_props_t     char_props,
                         ble_gatts_char_handles_t *char_handle,
                         ble_gatts_attr_md_t       m_attr_md,
                         uint16_t                  uuid)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify        = char_props.notify;
    char_md.char_props.write         = char_props.write;
    char_md.char_props.read          = char_props.read;
    char_md.char_props.write_wo_resp = char_props.write_wo_resp;
    char_md.char_props.indicate      = char_props.indicate;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = &cccd_md;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = uuid_type;
    ble_uuid.uuid = uuid;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = m_attr_md.rd_auth;
    attr_md.wr_auth = m_attr_md.wr_auth;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_PUBLIC_MAX_DATA_LEN;
    return sd_ble_gatts_characteristic_add(service_handle, &char_md, &attr_char_value, char_handle);
}

void fill_in_character_paraments(ble_gatt_char_props_t *p_char_props,
                                 ble_gatts_attr_md_t *  p_m_attr_md)
{
    memset(p_char_props, 0, sizeof(ble_gatt_char_props_t));
    memset(p_m_attr_md, 0, sizeof(ble_gatts_attr_md_t));
    p_char_props->read   = 1;
    p_char_props->write  = 1;
    p_char_props->notify = 1;

    p_m_attr_md->rd_auth = 0;
    p_m_attr_md->wr_auth = 0;
}
