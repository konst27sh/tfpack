//
// Created by sheverdin on 10/17/23.
//

#ifndef DM_CLIENT_SETTINGS_MODULE_H
#define DM_CLIENT_SETTINGS_MODULE_H

#include "dm_mainHeader.h"

#define SET_CMD_LEN             (128)

typedef enum
{
    SETTINGS_DEFAULT_ALL        = 0,
    SETTINGS_REBOOT_ALL         = 1,
    SETTINGS_SYS_RELOAD         = 2,
    SETTINGS_changes_show       = 3,
    SETTINGS_MAX_CMD            = 4
}SETTINGS_CMD;

typedef struct
{
    uint8_t setDHCP;
    uint8_t applyNetwork;
    uint8_t applySystem;
}SETTINGS_INFO_t;

typedef struct
{
    SETTINGS_CMD cmd;
    char  value[SETTINGS_VALUE_LENGTH];
}DYNAMIC_MAP_CMD;

typedef void (*runSettingsCmd_f)(void);

SETTINGS_INFO_t *getSettingInfo(void);
runSettingsCmd_f *getRunSettingsCmd(void);

#endif //DM_CLIENT_SETTINGS_MODULE_H
