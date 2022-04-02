#include <stdlib.h>

#include "msdp-net.h"

static void parse_packet(const char *buf, int len, fr_msdp_packet_t *packet);

static int write_packet(char *buf, fr_msdp_packet_t *packet);

mc_socket_t *m_socket(int type, struct multicast_settings *settings) {
  if (type < 0) {
    perror("Invalid type or settings are NULL");
    return NULL;
  }

  mc_socket_t *socket_ = (mc_socket_t *) malloc(sizeof(mc_socket_t));
  if (!socket_) {
    perror("Could not create socket object");
    return NULL;
  }

  socket_->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

  int reuse = 1;
  if (setsockopt(socket_->socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0) {
    perror("[newsocket()] : SO_REUSEADDR");
  }

  int loop = 1;
  if (setsockopt(socket_->socket_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (const char *)&loop, sizeof(loop)) < 0) {
    perror("[newsocket()] : IP_MULTICATS_LOOP");
  }

  if (settings->on_init) {
    settings->on_init();
  }

  return socket_;
}

int m_join_channel(mc_socket_t *socket) {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(MSDP_PORT);

  bind(socket->socket_fd, (struct sockaddr *)&addr, sizeof(addr));

  struct ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = inet_addr(MSDP_MULTICAST_IPv4);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  setsockopt(socket->socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq));
}

#define MCAST_ADDR(x) (x).sin_family = AF_INET; \
  (x).sin_addr.s_addr = inet_addr(MSDP_MULTICAST_IPv4); \
  (x).sin_port = htons(MSDP_PORT);

#define m_addr multicast_address.sin_addr.S_un.S_un_b

int m_receive(mc_socket_t *socket, fr_msdp_packet_t *packet) {
  if (!socket || !packet || socket->socket_fd < 0) return -2;

  socklen_t l = sizeof(socket->settings->multicast_address);
  char buf[256] = {0};

  int bytes = recvfrom(socket->socket_fd, buf, 256, 0, (struct sockaddr *)&(socket->settings->multicast_address), &l);
  if (bytes == SOCKET_ERROR) {
    return 0;
  }
  packet->from.name = inet_ntoa(socket->settings->multicast_address.sin_addr);
  packet->from.ui8_addr[0] = socket->settings->m_addr.s_b1;
  packet->from.ui8_addr[1] = socket->settings->m_addr.s_b2;
  packet->from.ui8_addr[2] = socket->settings->m_addr.s_b3;
  packet->from.ui8_addr[3] = socket->settings->m_addr.s_b4;

  parse_packet(buf, bytes, packet);
  if (socket->settings->on_rec) {
    socket->settings->on_rec(packet);
  }
  return bytes;
}

#undef m_addr

#define mcast_settings socket->settings->multicast_address

int m_send(mc_socket_t *socket, fr_msdp_packet_t *packet) {
  if (!socket || !packet || socket->socket_fd < 0) return -2;

  mcast_settings.sin_family = AF_INET;
  mcast_settings.sin_addr.S_un.S_addr = inet_addr(MSDP_MULTICAST_IPv4);
  mcast_settings.sin_port = htons(MSDP_PORT);

  socklen_t l = sizeof(mcast_settings);
  char buf[256] = {0};

  int len = write_packet(buf, packet);
  int bytes = sendto(socket->socket_fd, buf, len, 0, (struct sockaddr *)&mcast_settings, l);
  if (bytes == EAGAIN || bytes == EWOULDBLOCK) {
    return 0;
  }

  if (socket->settings->on_send) {
    socket->settings->on_send(packet);
  }
  return bytes;
}
#undef mcast_settings

static void parse_packet(const char *buf, int len, fr_msdp_packet_t *packet) {
  if (len < 27 || len > 256) return;

  int offset = 0;
  char tmp = buf[offset++];

  if (tmp < 0 || !PKT_IS_CODE(tmp)) {
    tmp = OP_ERR;
  }
  packet->code = tmp;

  packet->network_flags = MSDP_PKT_TO_UINT8(buf[offset++]);
  packet->net_id = MSDP_PKT_TO_UINT8(buf[offset++]);

  __arraycopy__(buf, offset, packet->reserved_bytes, 0, MSDP_PKT_RSVD_DEFINED_LEN);
  offset += MSDP_PKT_RSVD_DEFINED_LEN;

  packet->system = MSDP_PKT_TO_UINT8(buf[offset++]);
  packet->version = MSDP_PKT_TO_UINT8(buf[offset++]);
  packet->client_type = MSDP_PKT_TO_UINT8(buf[offset++]);

  __arraycopy__(buf, offset, packet->uid, 0, MSDP_PKT_UID_DEFINED_LEN);
  offset += MSDP_PKT_UID_DEFINED_LEN;

  unsigned char dlen = (unsigned char)buf[offset++];
  if (dlen > 0 && dlen <= MSDP_PKT_DATA_DEFINED_LEN) {
    __arraycopy__(buf, offset, packet->data, 0, dlen);
  }
  packet->data_len = dlen;
}

int write_packet(char *buf, fr_msdp_packet_t *packet) {
  int offset = 0;

  buf[offset++] = (char)packet->code;
  buf[offset++] = (char)packet->network_flags;
  buf[offset++] = (char)packet->net_id;

  __arraycopy__(packet->reserved_bytes, 0, buf, offset, MSDP_PKT_RSVD_DEFINED_LEN);
  offset += MSDP_PKT_RSVD_DEFINED_LEN;

  buf[offset++] = (char)packet->system;
  buf[offset++] = (char)packet->version;
  buf[offset++] = (char)packet->client_type;

  __arraycopy__(packet->uid, 0, buf, offset, MSDP_PKT_UID_DEFINED_LEN);
  offset += MSDP_PKT_UID_DEFINED_LEN;

  buf[offset++] = (char)packet->data_len;
  if (packet->data_len > 0 && packet->data_len <= 224) {
    __arraycopy__(packet->data, 0, buf, offset, packet->data_len);
    offset += packet->data_len;
  }
  return offset;
}

