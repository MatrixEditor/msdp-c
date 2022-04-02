//
// Created on 07.03.2022.
//
#include <stdio.h>

#define _TEST_CLIENT_
#undef _TEST_CLIENT_

#include "protocol/msdp-client.h"
#include "pi-client.h"

#ifdef _TEST_CLIENT_
  #include "protocol/msdp-packet-utils.h"
#endif

#ifdef _TEST_CLIENT_
static void test_client_send(void);
#else
static void test_server_receive(void);
#endif

int main(void) {
  if (setup_client() < 0) {
    print_i("warning -> could not setup client!");
    return -1;
  }

#ifdef _TEST_CLIENT_
  test_client_send();
#else
  test_server_receive();
#endif

  free_client();
  return 0;
}

#ifdef _TEST_CLIENT_
static void test_client_send(void) {
  fr_msdp_packet_t packet = {
          .data = {0},
          .reserved_bytes = {0},
          .uid = {0}
  };
  uint8_t nf = msdp_get_flags(1, 0, 0);

  msdp_fill_packet(&packet, OP_LVE, nf, 1, NULL, 1, 0, MSDP_PKT_CLIENT, ClientPool.msdp_uid(), 0, NULL);
  m_send(ClientPool.msdp_socket(), &packet);
}
#else
static void test_server_receive(void) {
  ClientPool.listen();
}
#endif