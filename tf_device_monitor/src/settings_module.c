//
// Created by sheverdin on 10/17/23.
//

#include "dm_mainHeader.h"
#include "settings_module.h"
#include "run_CLIcmd.h"

static void runReboot();
static void runReset();
static void runSystemRestart();
static void runChangesShow();
static void replaceTabsAndNewlines(char *str);

static SETTINGS_INFO_t settingsInfo =
{
    .setDHCP = 0,
    .applyNetwork = 0,
    .applySystem = 0
};

const char settings_commandTable[SETTINGS_MAX_CMD][SET_CMD_LEN] =
{
    {"firstboot -y && reboot"},
    {"reboot"},
    {"/etc/tf_clish/scripts/changes/config_changes.lua save"},
    {"/etc/tf_clish/scripts/changes/config_changes.lua show"}
};

runSettingsCmd_f runSetingsCmd[SETTINGS_MAX_CMD] =
{
    &runReset,
    &runReboot,
    &runSystemRestart,
    &runChangesShow
};

runSettingsCmd_f *getRunSettingsCmd(void)
{
    return runSetingsCmd;
}

SETTINGS_INFO_t *getSettingInfo(void)
{
    return &settingsInfo;
}

static void runReset()
{
    openlog("tf_dm", 0, LOG_USER);
    syslog(LOG_ALERT, "system reset to default");
    closelog();
    FILE *pipe = openPipe(settings_commandTable[SETTINGS_DEFAULT_ALL]);
    closePipe(pipe, "Reset");
}

static void runReboot()
{
    openlog("tf_dm", 0, LOG_USER);
    syslog(LOG_ALERT, "system reboot");
    closelog();
    FILE *pipe = openPipe(settings_commandTable[SETTINGS_REBOOT_ALL]);
    closePipe(pipe, "Reboot");
}

static void runSystemRestart()
{
    openlog("tf_dm", 0, LOG_USER);
    syslog(LOG_INFO, "settings changes");
    closelog();
    FILE *pipe = openPipe(settings_commandTable[SETTINGS_SYS_RELOAD]);
    char output[128];
    bzero(output, 128);
    while (fgets(output, OUTPUT_MAX_LENGTH, pipe))
    {
        replaceTabsAndNewlines(output);
        syslog(LOG_ALERT, "%s ", output);
    }
    closePipe(pipe, "runSystemRestart");
}

static void runChangesShow()
{
    openlog("tf_dm", 0, LOG_USER);
    closelog();
    FILE *pipe = openPipe(settings_commandTable[SETTINGS_changes_show]);
    //char output[128];
    //bzero(output, 128);
    //while (fgets(output, OUTPUT_MAX_LENGTH, pipe))
    //{
    //    replaceTabsAndNewlines(output);
    //    syslog(LOG_ALERT, "%s ", output);          // todo new change module
    //}
    closePipe(pipe, "runChangesShow");
}

static void replaceTabsAndNewlines(char *str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if ((str[i] == '\t') || (str[i] == '\n'))
        {
            str[i] = ' ';
            //i += 1;
        }
    }
}
