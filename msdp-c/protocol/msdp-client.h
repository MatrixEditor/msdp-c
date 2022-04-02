//
// Created by Leonard on 06.03.2022.
//

#ifndef PI_MSDP_MSDP_CLIENT_H
#define PI_MSDP_MSDP_CLIENT_H

#include "msdp-net.h"

struct cli_msdp_internal;

struct cli_msdp {

    void (*on_data)(const struct cli_msdp_internal *client, fr_msdp_packet_t *packet);

    void (*on_shutdown)(const struct cli_msdp_internal *client);
};

//#define startup_pool(...) startup_pool_helper((struct multicast_settings){__VA_ARGS__})
#define startup_pool_helper(x) ClientPool.startup((struct multicast_settings *)(x))

#define MAX_POOL_SIZE 5

extern const struct MSDP_CLIENT_API {

    void (*startup)(struct multicast_settings *settings);

    struct multicast_settings *(*settings)(void);

    void (*attach)(struct cli_msdp *client);

    void (*listen)(void);

    void (*terminate)(void);

    mc_socket_t *(*msdp_socket)(void);

    fr_byte_t *(*msdp_uid)(void);

} ClientPool;

#endif //PI_MSDP_MSDP_CLIENT_H
