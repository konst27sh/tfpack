//
// Created by BelyaevAlex on 18.12.2024.
//

#ifndef TFORTIS_PACKAGES_CONFIG_H
#define TFORTIS_PACKAGES_CONFIG_H

#include "main.h"

void load_port_config(struct uci_context *uci, struct uci_section *s);
void load_port_state_config(struct uci_context *uci, struct uci_section *s, int port);
void load_global_config(struct uci_context *uci, struct uci_section *s);
void config_load(int init);
void config_apply_quirks(struct config *config);

#endif //TFORTIS_PACKAGES_CONFIG_H
