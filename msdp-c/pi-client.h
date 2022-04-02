//
// Created on 07.03.2022.
//

#ifndef UNTITLED_PI_CLIENT_H
#define UNTITLED_PI_CLIENT_H

#include <stdio.h>

#include "protocol/msdp-packet.h"

#define print_i(msg) printf("[pi-client] (%s) : %s\n", __FILE__, msg)
#define print_l(msg, ...) printf(msg, __VA_ARGS__)

extern struct GLOBAL_SETTINGS {

    uint8_t net_id;

    //some more definitions in the future

} GlobalSettings;

int setup_client(void);

int free_client(void);

#endif //UNTITLED_PI_CLIENT_H
