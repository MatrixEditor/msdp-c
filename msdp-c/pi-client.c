//
// Created by Leonard on 07.03.2022.
//

#include <stdio.h>

#include "pi-client.h"
#include "protocol/msdp-client.h"
#include "protocol/msdp-log.h"
#include "protocol/msdp-packet-utils.h"

static void mc_cli_on_init(void);
static void mc_cli_on_send(const fr_msdp_packet_t *packet);
static void mc_cli_on_packet_received(const fr_msdp_packet_t *packet);
static void mc_cli_on_data(const struct cli_msdp_internal *client, fr_msdp_packet_t *packet);
static void mc_cli_on_shutdown(const struct cli_msdp_internal *client);

static void mc_cli_pkt_handle_error(const fr_msdp_packet_t *packet);
static void mc_cli_pkt_handle_not_impl(const fr_msdp_packet_t *packet);
static void mc_cli_pkt_handle_ini(const struct cli_msdp_internal *client, const fr_msdp_packet_t *packet);
static void mc_cli_pkt_handle_lve(const struct cli_msdp_internal *client, const fr_msdp_packet_t *packet);
static void mc_cli_pkt_handle_qry(const struct cli_msdp_internal *client, const fr_msdp_packet_t *packet);

static boolean_t cli_pkt_uid_cmp(const fr_byte_t *u0, const fr_byte_t *u1);

static struct sockaddr_in *msdp_mc_default_address = NULL;
static void mc_set_default_address(void);

static struct multicast_settings *msdp_mc_default_settings = NULL;
static void mc_set_multicast_settings(int type);

static struct hostent *host_entry = NULL;

struct GLOBAL_SETTINGS GlobalSettings = {
        .net_id  = MSDP_NET_DEFAULT_ID,
};

int setup_client(void) {
  log_init("pi-client.log");

#ifdef _WIN32
  DO_WSAStartup
  log_info("WSA-Startup done!");
#endif

  mc_set_default_address();
  mc_set_multicast_settings(0);

  startup_pool_helper(msdp_mc_default_settings);
  log_info("ClientManager started");

  struct cli_msdp *handler = (struct cli_msdp *)malloc(sizeof(struct cli_msdp));
  handler->on_data = &mc_cli_on_data;
  handler->on_shutdown = &mc_cli_on_shutdown;
  ClientPool.attach(handler);

  return 0;
}

int free_client(void) {
  log_info("Closing client with all components!");
  return 0;
}

#define CASE(exp, ...) case exp: \
  __VA_ARGS__; break;

#define CASE_HELPER(c, exp, ...) c exp: \
  __VA_ARGS__; break;

static void mc_cli_on_data(const struct cli_msdp_internal *client, fr_msdp_packet_t *packet) {
  if (!packet || !client) {
    log_error("packet is null or client is null!");
    return;
  }

  if (cli_pkt_uid_cmp(ClientPool.msdp_uid(), packet->uid)) {
    log_info("Collected self-sent packet -> deleted!");
    return;
  }

  switch (packet->code) {
    CASE(OP_INI, mc_cli_pkt_handle_ini(client, packet))
    CASE(OP_ERR, mc_cli_pkt_handle_error(packet))
    CASE(OP_QRY, mc_cli_pkt_handle_qry(client, packet))
    CASE(OP_LVE, mc_cli_pkt_handle_lve(client, packet))

    CASE_HELPER(default, , mc_cli_pkt_handle_not_impl(packet))
  }
}

static void mc_cli_pkt_handle_ini(const struct cli_msdp_internal *client, const fr_msdp_packet_t *packet) {
  if (!packet || !client) {
    log_error("packet is null or client is null!");
    return;
  }

  log_info("Received an ini-packet: a new host will be registered on the channel.");
#ifdef _DEBUG
  log_debug("Packet data:\n\tnetid=%d, uid=%s", packet->net_id, packet->uid);
#endif
}

static void mc_cli_pkt_handle_qry(const struct cli_msdp_internal *client, const fr_msdp_packet_t *packet) {
  if (!host_entry) {
    char hbuf[256] = {0};
    if (gethostname(hbuf, sizeof(hbuf)) < 0) {
      log_warn("Couldn't find host-name");
      return;
    }
    host_entry = gethostbyname(hbuf);
  }

  if (!MSDP_PKT_IS_CALLBACK(packet->network_flags)) {
    log_warn("Received qry-packet without a callback-flag! (note: no answer will be sent)");
    return;
  }

  if (packet->net_id != MSDP_NET_DEFAULT_ID) {
    log_warn("Received a non-standard net_id. This could lead to errors while interpreting (net_id=%d)",
             packet->net_id);
  }

  fr_msdp_packet_t p = {.data = {0},.uid = {0},.reserved_bytes = {0}};

  fr_byte_t data_buf[17] = {0};

  __arraycopy__(inet_ntoa(*(struct in_addr *)host_entry->h_addr_list[0]), 0, data_buf, 0, 14);
  data_buf[15] = 45; //port should be set here
  data_buf[16] = 45;

  uint8_t nf = msdp_get_flags(0, 0, 0);
  msdp_fill_packet(&p, OP_RSP, nf, GlobalSettings.net_id, NULL,
                   MSDP_SYSTEM, __msdp__, msdp_mc_default_settings->type,
                   ClientPool.msdp_uid(), 17, data_buf);

  m_send(ClientPool.msdp_socket(), &p);
}

static void mc_cli_pkt_handle_lve(const struct cli_msdp_internal *client, const fr_msdp_packet_t *packet) {
  log_info("Received a LEAVE-packet -> shutdown now.");
  ClientPool.terminate();
}

static void mc_cli_on_init(void) {
  log_info("Multicast-socket initialized without errors!");
}

static void mc_cli_on_send(const fr_msdp_packet_t *packet) {
  if (packet->code == OP_ERR) {
    log_warn("Error detected! Detailed message follows:\n\tdata : %s", packet->data);
  }
  log_info("packet sent to multicast address, code : %d", packet->code);
}

static void mc_cli_on_packet_received(const fr_msdp_packet_t *packet) {
  log_info("packet received from %s, code=%d, len=%d", packet->from.name, packet->code, packet->data_len);
}

static void mc_cli_on_shutdown(const struct cli_msdp_internal *client) {
  log_info("Shutting client down!");
}

static void mc_cli_pkt_handle_not_impl(const fr_msdp_packet_t *packet) {
  log_trace("the received packet code is not implemented yet!");
}

static void mc_cli_pkt_handle_error(const fr_msdp_packet_t *packet) {
  log_info("[err] in packet:\n\tuid=%s, data_len=%d\n\tdata='%s'", packet->uid, packet->data_len, packet->data);
}

static void mc_set_multicast_settings(int type) {
  if (msdp_mc_default_settings || !msdp_mc_default_address) {
    return;
  }

  msdp_mc_default_settings = (struct multicast_settings *) malloc(sizeof(struct multicast_settings));
  msdp_mc_default_settings->multicast_address = *msdp_mc_default_address;
  msdp_mc_default_settings->on_send = &mc_cli_on_send;
  msdp_mc_default_settings->on_rec = &mc_cli_on_packet_received;
  msdp_mc_default_settings->on_init = &mc_cli_on_init;
  msdp_mc_default_settings->type = type;
}

static void mc_set_default_address(void) {
  if (msdp_mc_default_address) {
    return;
  }

  msdp_mc_default_address = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
  msdp_mc_default_address->sin_family = AF_INET;
  msdp_mc_default_address->sin_addr.S_un.S_addr = inet_addr(MSDP_MULTICAST_IPv4);
  msdp_mc_default_address->sin_port = htons(MSDP_PORT);
}

static boolean_t cli_pkt_uid_cmp(const fr_byte_t *u0, const fr_byte_t *u1) {
  if (!u0 || !u1) return 0;
  for (int i = 0; i < MSDP_PKT_UID_DEFINED_LEN; ++i) {
    if (u0[i] != u1[i]) {
      return 0;
    }
  }
  return 1;
}