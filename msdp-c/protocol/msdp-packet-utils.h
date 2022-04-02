//
// Created on 07.03.2022.
//

#ifndef _MSDP_PACKET_UTILS_H_
#define _MSDP_PACKET_UTILS_H_

#include "msdp-packet.h"

/**
 * /brief
 * Small wrapper type to make function parameters easier to read.
 */
typedef int boolean_t;


#define ini_packet(packet, f, n, r, s, v, cl, u, dl, d) \
  msdp_fill_packet(packet, OP_INI, f, n, r, MSDP_SYSTEM, v, cl, u, dl, d)

#define err_packet(packet, f, n, r, s, v, cl, u, dl, d) \
  msdp_fill_packet(packet, OP_ERR, f, n, r, MSDP_SYSTEM, v, cl, u, dl, d)


/**
 * /brief
 * Utility function to place all attributes related to the given packet. All parameters
 * should be set. For more information about the different fields refer to the struct
 * commenting.
 */
void msdp_fill_packet(fr_msdp_packet_t *packet,fr_msdp_code_e code,uint8_t network_flags,uint8_t net_id,
                      fr_byte_t *rsvd_bytes, uint8_t system,uint8_t version,uint8_t client_type,
                      fr_byte_t *uid,uint8_t data_len, fr_byte_t *data);


uint8_t msdp_get_flags(boolean_t has_callback, boolean_t is_virtual, boolean_t rsvd_enabled);

uint8_t msdp_get_client_type(boolean_t server, boolean_t client, boolean_t daemon);

#endif //_MSDP_PACKET_UTILS_H_
