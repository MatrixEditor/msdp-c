//
// Created on 07.03.2022.
//

#include "msdp-packet-utils.h"
#include "msdp-net.h"

uint8_t msdp_get_flags(boolean_t has_callback, boolean_t is_virtual, boolean_t rsvd_enabled) {
  uint8_t network_flags = 0;

  if (has_callback) {
    MSDP_PKT_FLAG_ADD(network_flags, MSDP_PKT_CALLBACK);
  }
  if (is_virtual) {
    MSDP_PKT_FLAG_ADD(network_flags, MSDP_PKT_VIRTUALN);
  }
  if (rsvd_enabled) {
    MSDP_PKT_FLAG_ADD(network_flags, MSDP_PKT_RSVD_ACT);
  }

  return network_flags;
}

uint8_t msdp_get_client_type(boolean_t server, boolean_t client, boolean_t daemon) {
  uint8_t client_type = 0;

  if (server) {
    MSDP_PKT_FLAG_ADD(client_type, MSDP_PKT_SERVER);
  }
  if (client) {
    MSDP_PKT_FLAG_ADD(client_type, MSDP_PKT_CLIENT);
  }
  if (daemon) {
    MSDP_PKT_FLAG_ADD(client_type, MSDP_PKT_DAEMON);
  }

  return client_type;
}

void msdp_fill_packet(fr_msdp_packet_t *packet, fr_msdp_code_e code, uint8_t network_flags, uint8_t net_id,
                      fr_byte_t *rsvd_bytes, uint8_t system, uint8_t version, uint8_t client_type,
                      fr_byte_t *uid, uint8_t data_len, fr_byte_t *data) {
  if (!packet || !PKT_IS_CODE(code)) {
    return;
  }
  packet->code = code;
  packet->network_flags = network_flags;
  packet->net_id = net_id;

  if (rsvd_bytes) {
    __arraycopy__(rsvd_bytes, 0, packet->reserved_bytes, 0, 12);
  }

  packet->system = system;
  packet->version = version;
  packet->client_type = client_type;

  if (uid) {
    __arraycopy__(uid, 0, packet->uid, 0, 10);
  }

  packet->data_len = data_len;
  if (data_len > 0 && data_len < 224 && data) {
    __arraycopy__(data, 0, packet->data, 0, data_len);
  }
}