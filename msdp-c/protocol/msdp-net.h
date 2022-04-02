#ifndef PI_UPNP_MSDP_NET_H
#define PI_UPNP_MSDP_NET_H

#ifdef _WIN32
  #pragma comment(lib, "ws2_32.lib")

  #include <Winsock2.h> // before Windows.h, else Winsock 1 conflict
  #include <Ws2tcpip.h> // needed for ip_mreq definition for multicast
  #include <Windows.h>
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
#endif

#ifdef _WIN32
  #define DO_WSAStartup WSADATA wsaData; \
    if (WSAStartup(0x0101, &wsaData)) { \
      printf("WSAStartup"); \
      exit(0); \
    } \

  #define EXIT_WSA WSACleanup();
#else
  #define DO_WSAStartup
  #define EXIT_WSA
#endif

#ifdef __cplusplus
  #define __arraycopy__(src, srcPos, dest, destPos, length) std::copy(src+(srcPos), src+(length), dest+(destPos))
#else
  #define __arraycopy__(src, srcPos, dest, destPos, length) memcpy((void *)dest+(destPos), (const void *)src+(srcPos), (size_t)(length))
#endif

#include "msdp-packet.h"

/**
 * The actual UDP-Broadcast address used to send and receive packets. Usually
 * no IPv6 is implemented but could be done in a future release.
 *
 * @since 1.0
 */
#define MSDP_MULTICAST_IPv4 "224.0.0.200"

/**
 * The pre-defined standard port for sending and receiving MSDP packets. This
 * number shouldn't be changing in future releases, because some errors
 * with older versions could be the consequence.
 *
 * @since 1.0
 */
#define MSDP_PORT 3439

/**
 * /brief
 * A class to store different options for the multicast socket.
 */
struct multicast_settings {

    /**
     * /brief
     * The type of socket (SERVER, CLIENT or DAEMON)
     */
    int type;

    /**
     * /brief
     * The multicast address object to connect and send to
     */
    struct sockaddr_in multicast_address;

    /**
     * /brief
     * This method is called if the socket is successfully created.
     */
    void (*on_init)(void);

    /**
     * /brief
     * Called when data had been send to the multicast address.
     */
    void (*on_send)(const fr_msdp_packet_t *packet);

    /**
     * /brief
     * Called when a packet has been received.
     */
    void (*on_rec)(const fr_msdp_packet_t *packet);
};

/**
 * /brief
 * The base type for creating multicast sockets. It uses the multicast_settings
 * type to infer different options.
 */
typedef struct mc_multicast_socket_container {
  struct multicast_settings *settings;

  SOCKET socket_fd;

  //maybe some listener methods here
} mc_socket_t;

#define new_multicast_socket(i, ...) m_socket(i, (struct multicast_settings){__VA_ARGS__})

#define close_multicast_socket(socket) closesocket(socket)

/**
 * /brief
 * Creates a new multicast socket associated with the given settings.
 */
mc_socket_t *__cdecl m_socket(int type, struct multicast_settings *settings);

/**
 * /brief
 * Adds a membership to the multicast group.
 *
 * @param socket the multicast socket
 * @returns true if no errors occurred
 */
int __cdecl m_join_channel(mc_socket_t *socket);

/**
 * /brief
 * Receives the next packet into the pointer.
 *
 * @param socket the multicast socket reference
 * @param packet the packet reference where data should be written into
 * @returns the amount of received bytes
 */
int __cdecl m_receive(mc_socket_t *socket, fr_msdp_packet_t *packet);

/**
 * /brief
 * Wraps the given packet into bytes and sends them to the multicast channel.
 *
 * @param socket the multicast socket reference
 * @param packet the data which will be send
 * @returns the amount of bytes sent
 */
int __cdecl m_send(mc_socket_t *socket, fr_msdp_packet_t *packet);

#endif //PI_UPNP_MSDP_NET_H
