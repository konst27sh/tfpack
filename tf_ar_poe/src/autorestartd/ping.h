//
// Created by sheverdin on 7/2/24.
//

#ifndef TF_AR_POE_PING_H
#define TF_AR_POE_PING_H

#include "port_module.h"

int ping(port_config_info_t *portConfigInfo, unsigned long timeout);
void sendPingRequest(port_config_info_t *portConfigInfo);


#endif //TF_AR_POE_PING_H
