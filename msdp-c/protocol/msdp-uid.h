//
// Created on 07.03.2022.
//

#ifndef _PI_MSDP_UID_H_
#define _PI_MSDP_UID_H_

#include <time.h>
#include <stdlib.h>

#include "msdp-packet.h"

#define random(min, max) ((rand() % (int)(((max) + 1) - (min))) + (min))

#define init_random() srand(time(NULL))

#define gen_uuid(u) \
  for (int i = 0; i < MSDP_PKT_UID_DEFINED_LEN; i++) { \
    u[i] = random(0, 16); \
  }

#endif //_PI_MSDP_UID_H_
