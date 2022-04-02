//
// Created on 06.03.2022.
//
#include <stdlib.h>

#include "msdp-client.h"
#include "msdp-log.h"
#include "msdp-uid.h"

struct cli_msdp_internal {
    struct multicast_settings *settings;
    struct cli_msdp *cli_listener_map[MAX_POOL_SIZE];

    mc_socket_t *socket;

    fr_byte_t shared_uid[10];

    int interrupt;

    unsigned current_size;
    unsigned free_space;
};

static void cli_pool_startup(struct multicast_settings *settings);

static struct multicast_settings *cli_pool_get_settings(void);

static fr_byte_t *cli_msdp_shared_uid(void);

static mc_socket_t *cli_msdp_socket(void);

static void cli_pool_attach(struct cli_msdp *client);

static void cli_pool_listen(void);

static void cli_pool_terminate(void);

const struct MSDP_CLIENT_API ClientPool = {
        .settings = &cli_pool_get_settings,
        .attach = &cli_pool_attach,
        .listen = &cli_pool_listen,
        .terminate = &cli_pool_terminate,
        .startup = &cli_pool_startup,
        .msdp_socket = &cli_msdp_socket,
        .msdp_uid = &cli_msdp_shared_uid
};

struct cli_msdp_internal ClientManager = {
        .settings = NULL,
        .interrupt = 0,
        .current_size = 0,
        .free_space = MAX_POOL_SIZE,
        .shared_uid = {0}
};

static struct multicast_settings *cli_pool_get_settings(void) {
  return ClientManager.settings;
}

static void cli_pool_terminate(void) {
  ClientManager.interrupt = 1;
}

static void cli_pool_attach(struct cli_msdp *client) {
  if (!client || ClientManager.free_space == 0){
    log_warn("Tried to add a new client handler to a full pool!");
    return;
  }
  ClientManager.cli_listener_map[ClientManager.current_size++] = client;
  ClientManager.free_space = MAX_POOL_SIZE - ClientManager.current_size;
}

static void cli_pool_startup(struct multicast_settings *settings) {
  if (ClientManager.socket != NULL) {
    log_warn("Socket is null - aborting startup of ClientPool!");
    return;
  }

  mc_socket_t *socket = m_socket(settings->type, settings);
  socket->settings = settings;
  ClientManager.socket = socket;
  ClientManager.settings = settings;
  ClientManager.interrupt = 0;

  //the randomness should be increased in the future - for now it's okay
  init_random();
  gen_uuid(ClientManager.shared_uid)

  if (m_join_channel(socket) < 0) {
    log_error("[join-channel()] : could not join channel!");
  }
}

static void cli_pool_listen(void) {
  if (ClientManager.socket == NULL || ClientManager.interrupt) {
    return;
  }

  fr_msdp_packet_t dummy_packet = {
          .data = {0},
          .uid = {0},
          .reserved_bytes = {0}
  };
  unsigned internal_counter = 0;
  do {
    if (m_receive(ClientManager.socket, &dummy_packet) <= 0) {
      log_fatal("Received -1 or 0 bytes - exit after 10 tries : %d/10", internal_counter++);
      continue;
    }

    for (int i = 0; i < ClientManager.current_size; i++) {
      if (!ClientManager.cli_listener_map[i]->on_data) continue;
      struct cli_msdp* handler = ClientManager.cli_listener_map[i];

      handler->on_data(&ClientManager, &dummy_packet);
    }
    if (internal_counter > 0){
      internal_counter = 0;
    }
  } while (!ClientManager.interrupt && internal_counter < 10);

  for (int i = 0; i < ClientManager.current_size; i++) {
    if (!ClientManager.cli_listener_map[i]->on_data) continue;
    ClientManager.cli_listener_map[i]->on_shutdown(&ClientManager);
  }

}

static mc_socket_t *cli_msdp_socket(void) {
  return ClientManager.socket;
}

static fr_byte_t *cli_msdp_shared_uid(void) {
  return ClientManager.shared_uid;
}


