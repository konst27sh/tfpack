//
// Created by sheverdin on 10/11/23.
//

#include "state_handler.h"
#include "net_utils.h"
#include "run_CLIcmd.h"
#include "settings_module.h"

static event_FillMsg_Callback_t eventFillMsg_CallbackArray[CLIENT_FILLMSG_NUM_OF_CALLBACKS];
static event_SendMsg_Callback_t eventSendMsg_CallbackArray[CLIENT_SENDMSG_NUM_OF_CALLBACKS];

static error_code_t client_emitEvent_fillmsg(void);
static error_code_t client_registerEvent_fillmsg(event_FillMsg_Callback_t func_fillMsgCallback);
static error_code_t client_unRegisterEvent_fillmsg(event_FillMsg_Callback_t func_fillMsgCallback);

static error_code_t client_emitEvent_sendmsg(void);
static error_code_t client_registerEvent_sendmsg(event_SendMsg_Callback_t func_fillMsgCallback);
static error_code_t client_unRegisterEvent_sendmsg(void);

static error_code_t client_handler(void);
static CLIENT_EVENT_e client_waitCommand(void);
static  error_code_t client_sendSearchMsg(void);
static  error_code_t client_sendSettMsg(void);
static error_code_t client_settings(void);

static pass_id_e checkMacAddress(void);
static error_code_t checkMD5Sum(void);

static void mainTest(search_out_msg_t *searchMsg);

static uint8_t mainTestFlag = 0;

search_out_msg_t searchOutMsg;
SET_SETTINGS_MSG settingsMsg;
sett_msg_t settMsg;
pass_id_e passFlag = 0;

clientInfo_t clientInfo =
{
    .state = CLIENT_SATE_INIT,
    .event = CLIENT_EVENT_IDLE,
    .count = 0,
    .msgType = type_idle
};

error_code_t client_app(void)
{
    error_code_t errorCode = ERR_OK;
    openlog("tf_dm", 0, LOG_USER);
    while((errorCode ^= ERR_CONFIGURATION)
            && (errorCode ^= ERR_NETWORK) )
    {
        errorCode = client_handler();
        //if(errorCode)
        //{
        //    syslog(LOG_ERR, "error: %d", errorCode);
        //}
    }
    return errorCode;
}

static error_code_t client_handler(void)
{
    openlog("tf_dm", 0, LOG_USER);
    error_code_t errorCode = ERR_OK;
    switch (clientInfo.state)
    {
        case CLIENT_SATE_IDLE:
        {

        }
        break;

        case CLIENT_SATE_INIT:
        {
            errorCode = net_socketInit();
            client_registerEvent_fillmsg(&fillNetInfo);
            client_registerEvent_fillmsg(&getTimeInfo);
            client_registerEvent_fillmsg(&getBoardInfo);
            client_registerEvent_fillmsg(&getSystemInfo);
            clientInfo.state = CLIENT_STATE_WAIT;
        }
        break;

        case CLIENT_STATE_WAIT:
        {
            clientInfo.event |= client_waitCommand();
            if (clientInfo.event & CLIENT_EVENT_FIND)
            {
                clientInfo.msgType = type_searchMsg;
                clientInfo.event &= (~CLIENT_EVENT_FIND);
                clientInfo.state = CLIENT_STATE_FILL_MSG;
                clientInfo.count = 0;
            }

            if (clientInfo.event & CLIENT_EVENT_SETTINGS)
            {
                clientInfo.msgType = type_settingsMsg;
                clientInfo.event &= (~CLIENT_EVENT_SETTINGS);
                clientInfo.state = CLIENT_STATE_SETTINGS;
                clientInfo.count = 0;
            }
        }
        break;

        case CLIENT_STATE_SEND:
        {
            clientInfo.state = CLIENT_STATE_RELOAD;    // TODO check this state
            errorCode |= client_emitEvent_sendmsg();
            errorCode |= client_unRegisterEvent_sendmsg();
            return errorCode;
        }
        break;

        case CLIENT_STATE_FILL_MSG:
        {
            client_emitEvent_fillmsg();
            searchOutMsg.struct1.msg_type = (uint8_t) ack_find_device;
            if (clientInfo.event & CLIENT_EVENT_MSG_READY)
            {
                clientInfo.event &= (~CLIENT_EVENT_MSG_READY);
                client_registerEvent_sendmsg(client_sendSearchMsg);
                clientInfo.state = CLIENT_STATE_SEND;
            }
        }
        break;

        case CLIENT_STATE_SETTINGS:
        {
            errorCode = client_settings();
            for (int i = 0; i < MD5_LEN; i++)
            {
                settingsMsg.settMsg.settings_struct.pass_md5[i] = 0;
            }
            settingsMsg.settMsg.settings_struct.msg_type = (uint8_t) ack_to_set_config;
            switch (errorCode)
            {
                case ERR_AUTHENTICATION:
                {
                    syslog(LOG_ERR, "error: authentication");
                    client_registerEvent_sendmsg(client_sendSettMsg);
                    settingsMsg.settMsg.settings_struct.pass_md5[0] = 1;
                    clientInfo.state = CLIENT_STATE_SEND;
                    clientInfo.event = 0;
                }
                break;

                case ERR_OK:
                {
                    client_registerEvent_sendmsg(client_sendSettMsg);
                    settingsMsg.settMsg.settings_struct.pass_md5[0] = 0;
                    clientInfo.state = CLIENT_STATE_SEND;
                    clientInfo.event |= CLIENT_EVENT_SYS_RELOAD;
                }
                break;

                case ERR_REBOOT:
                {
                    client_registerEvent_sendmsg(client_sendSettMsg);
                    settingsMsg.settMsg.settings_struct.pass_md5[0] = 0;
                    clientInfo.state = CLIENT_STATE_SEND;
                    clientInfo.event |= CLIENT_EVENT_REBOOT;
                }
                break;

                case ERR_DEFAULT:
                {
                    client_registerEvent_sendmsg(client_sendSettMsg);
                    settingsMsg.settMsg.settings_struct.pass_md5[0] = 0;
                    clientInfo.state = CLIENT_STATE_SEND;
                    clientInfo.event |= CLIENT_EVENT_DEFAULT;
                }
                break;

                case INFO_MAC_NOT_IDENTICAL:
                    syslog(LOG_INFO, "error: mac not identical");
                    clientInfo.state = CLIENT_STATE_WAIT;
                break;
            }
        }
        break;

        case CLIENT_STATE_RELOAD:
        {
            runSettingsCmd_f *runSettingsCmd = getRunSettingsCmd();
            if (clientInfo.event & CLIENT_EVENT_SYS_RELOAD)
            {
                clientInfo.event &= (~CLIENT_EVENT_SYS_RELOAD);
                runSettingsCmd[SETTINGS_changes_show]();
                runSettingsCmd[SETTINGS_SYS_RELOAD]();
            }
            if (clientInfo.event & CLIENT_EVENT_REBOOT)
            {

                clientInfo.event &= (~CLIENT_EVENT_REBOOT);
                runSettingsCmd[SETTINGS_REBOOT_ALL]();
            }
            if (clientInfo.event & CLIENT_EVENT_DEFAULT)
            {
                clientInfo.event &= (~CLIENT_EVENT_DEFAULT);
                runSettingsCmd[SETTINGS_DEFAULT_ALL]();
            }
            clientInfo.state = CLIENT_STATE_WAIT;
        }
        break;
    }
    closelog();
    return errorCode;
}

static  error_code_t client_sendSettMsg(void)
{
    char *bufferAddress = getBufferAddress();
    memcpy(bufferAddress, settingsMsg.settMsg.buff, sizeof(settingsMsg.settMsg.buff));
    int error = net_sendMsg();
    return error;
}

static  error_code_t client_sendSearchMsg(void)
{
    char *bufferAddress = getBufferAddress();
    memcpy(bufferAddress, searchOutMsg.buff, sizeof(searchOutMsg.buff));
    int error = net_sendMsg();

    //if (mainTestFlag)
    //{
        //mainTest(&searchOutMsg);
    //}
    return error;
}

static CLIENT_EVENT_e client_waitCommand(void)
{
    uint8_t cmd = 0;
    uint8_t data[MAX_BUFFER_SIZE];

    cmd = net_getCommand();
    CLIENT_EVENT_e event = CLIENT_EVENT_IDLE;

    switch(cmd)
    {
        case request_find_device:
        {
            event = CLIENT_EVENT_FIND;
        }
        break;

        case request_set_config:
        {
            event = CLIENT_EVENT_SETTINGS;
        }
        break;

        case request_to_update:
        {
            event = CLIENT_EVENT_UPDATE;
        }
        break;


        case ack_to_update:
        case ack_find_device:
        case ack_to_set_config:
        {
            event = CLIENT_EVENT_IDLE;
        }
        break;

        default:
        {
            openlog("tf_dm", 0, LOG_USER);
            syslog(LOG_ERR, "error: received command is wrong");
            closelog();
            event = CLIENT_EVENT_IDLE;
        }
    }
    return event;
}

static error_code_t client_settings(void)
{
    error_code_t errorCode = ERR_OK;
    pass_id_e testID = 0;

    passFlag |= pass_mac;
    passFlag |= pass_md5;
    getBufferLen((size_t*)&settingsMsg.len);
    bzero(settingsMsg.settMsg.buff, sizeof(settingsMsg.settMsg.buff));
    getBuffer((uint8_t*)&settingsMsg.settMsg.buff, 0);

    passFlag |= checkMacAddress();
    if (passFlag & pass_mac)
    {
        errorCode = checkMD5Sum();
    }
    else
    {
        errorCode = INFO_MAC_NOT_IDENTICAL;
    }
    return errorCode;
}

static pass_id_e checkMacAddress(void)
{
    //getNetInfo(&searchOutMsg);
    uint8_t  macAddress[MAC_LEN] = {0, 0, 0, 0, 0, 0};

    for (int i=0; i < MAC_LEN; i++)
    {
        if ((uint8_t) searchOutMsg.struct1.mac[i] != (uint8_t) settingsMsg.settMsg.settings_struct.mac[i])
        {
            i = MAC_LEN;
            passFlag &= (~pass_mac);
        }
    }
    return passFlag;
}

static error_code_t checkMD5Sum(void)
{
    error_code_t errorCode = ERR_OK;
    openlog("tf_dm", 0, LOG_USER);
    char output[128];
    char hash_dm[33];
    char msg_dm[512];
    hash_dm[32] = '\0';
    strcpy(output, "");
    strcpy(hash_dm, "");
    bzero(msg_dm, 512);
    FILE *userConfig = NULL;

    strncpy(hash_dm, settingsMsg.settMsg.settings_struct.pass_md5, 32);
    uint16_t count = 0;
    for (int i = 0; i<settingsMsg.len; i++)
    {
        if (settingsMsg.settMsg.settings_struct.msg[i] != 0)
        {
            msg_dm[count] = settingsMsg.settMsg.settings_struct.msg[i];
            count++;
        }
    }
    char ifname[16];
    bzero(ifname, 16);
    get_ifName(ifname);

    char checkHash_script[] = "/etc/tf_device_monitor_scripts/user_handler.lua ";
    char cmd_checkHash[512];
    strcpy(cmd_checkHash, "");
    strcat(cmd_checkHash, checkHash_script);
    strcat(cmd_checkHash, " ");
    strcat(cmd_checkHash, hash_dm);
    strcat(cmd_checkHash, " \"");
    strcat(cmd_checkHash, msg_dm);
    strcat(cmd_checkHash, "\" ");
    strcat(cmd_checkHash, " ");
    strcat(cmd_checkHash, ifname);
    userConfig = openPipe(cmd_checkHash);
    while (fgets(output, OUTPUT_MAX_LENGTH, userConfig))
    {
        if (strcmp(output, "hash_error\n") == 0)
        {
            //syslog(LOG_DEBUG, "error: hash");
            errorCode = ERR_AUTHENTICATION;
            passFlag &= (~pass_md5);
        }
        else if (strcmp(output, "msg_error\n") == 0)
        {
            syslog(LOG_ERR, "error: message");
            errorCode = ERR_AUTHENTICATION;
            passFlag &= (~pass_md5);
        }
        else if (strcmp(output, "config_error\n") == 0)
        {
            syslog(LOG_ERR, "error: config");
            errorCode = ERR_AUTHENTICATION;
            passFlag &= (~pass_md5);
        }
        else if (strcmp(output, "config_reboot\n") == 0)
        {
           errorCode = ERR_REBOOT;
        }
        else if (strcmp(output, "config_default\n") == 0)
        {
             errorCode = ERR_DEFAULT;
        }
    }
    closePipe(userConfig, cmd_checkHash);
    return errorCode;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

static error_code_t client_emitEvent_fillmsg(void)
{
    if(clientInfo.count < CMD_MAX)
    {
        if (eventFillMsg_CallbackArray[clientInfo.count] != NULL)
        {
            eventFillMsg_CallbackArray[clientInfo.count](&searchOutMsg);
        }
        clientInfo.count++;
    }
    else if (clientInfo.count >= CMD_MAX)
    {
        clientInfo.event |= CLIENT_EVENT_MSG_READY;
    }
    return ERR_OK;
}

static error_code_t client_registerEvent_fillmsg(event_FillMsg_Callback_t func_fillMsgCallback)
{
    for (int i = 0 ; i < CLIENT_FILLMSG_NUM_OF_CALLBACKS ; i++)
    {
        if (eventFillMsg_CallbackArray[i] == NULL)
        {
            eventFillMsg_CallbackArray[i] = func_fillMsgCallback;
            return ERR_OK;
        }
    }
    return ERR_UNAVAILABLE_RESOURCE;
}

static error_code_t client_unRegisterEvent_fillmsg(event_FillMsg_Callback_t func_fillMsgCallback)
{
    for (int i = 0 ; i < CLIENT_FILLMSG_NUM_OF_CALLBACKS ; i++)
    {
        if (eventFillMsg_CallbackArray[i] == func_fillMsgCallback)
        {
            eventFillMsg_CallbackArray[i] = NULL;
            return ERR_OK;
        }
    }
    return ERR_OBJ_NOT_FOUND;
}

///=======================================================================================================
static error_code_t client_emitEvent_sendmsg(void)
{
    if (eventSendMsg_CallbackArray[clientInfo.msgType] != NULL)
    {
        eventSendMsg_CallbackArray[clientInfo.msgType]();
        return ERR_OK;
    }
    return ERR_OBJ_NOT_FOUND;
}

static error_code_t client_registerEvent_sendmsg(event_SendMsg_Callback_t func_fillMsgCallback)
{
    if (eventSendMsg_CallbackArray[clientInfo.msgType] == NULL)
    {
        eventSendMsg_CallbackArray[clientInfo.msgType] = func_fillMsgCallback;
        return ERR_OK;
    }
    return ERR_UNAVAILABLE_RESOURCE;
}

static error_code_t client_unRegisterEvent_sendmsg(void)
{
    if (eventSendMsg_CallbackArray[clientInfo.msgType] != NULL)
    {
        eventSendMsg_CallbackArray[clientInfo.msgType] = NULL;
        return ERR_OK;
    }
    return ERR_OBJ_NOT_FOUND;
}

static void mainTest(search_out_msg_t *searchMsg)
{
    openlog("tf_dm", LOG_PID, LOG_USER);
    syslog(LOG_NOTICE, "## DEVICE TYPE: = %d", searchMsg->struct1.dev_type);
    printf("##   DEVICE TYPE: %d\n", searchMsg->struct1.dev_type);
    printf("##   IP: \t");
    syslog(LOG_NOTICE, "## IP: %d.%d.%d.%d",
           searchMsg->struct1.ip[0],
           searchMsg->struct1.ip[1],
           searchMsg->struct1.ip[2],
           searchMsg->struct1.ip[3]);
    for (int i = 0; i < IPV4_LEN; i++)
    {

        printf("%d", searchMsg->struct1.ip[i]);
        printf(" ");
    }
    printf("\n");

    syslog(LOG_NOTICE,"## MAC: %X:%X:%X:%X:%X:%X", searchMsg->struct1.mac[0],
            searchMsg->struct1.mac[1],
            searchMsg->struct1.mac[2],
            searchMsg->struct1.mac[3],
            searchMsg->struct1.mac[4],
            searchMsg->struct1.mac[5]);
    for (int i = 0; i < MAC_LEN; i++)
    {
        printf("%d", searchMsg->struct1.mac[i]);
        printf(" ");
    }

    printf("\n");
    printf("##   DESCRIPTION: \n");
    printf("-> %s\n", searchMsg->struct1.dev_descr);
    syslog(LOG_NOTICE, "## DESCRIPTION: %s", searchMsg->struct1.dev_descr);
    printf("##   LOCATION: \n");
    printf("-> %s\n", searchMsg->struct1.dev_loc);
    syslog(LOG_NOTICE, "## LOCATION: %s", searchMsg->struct1.dev_loc);
    printf("##   UP_TIME: \t");
    uint32_t uptime = 0;
    printf(" %x ", searchMsg->struct1.uptime[3]);
    printf(" %x ", searchMsg->struct1.uptime[2]);
    printf(" %x ", searchMsg->struct1.uptime[1]);
    printf(" %x", searchMsg->struct1.uptime[0]);

    uptime |= (searchMsg->struct1.uptime[3] & 0xFFFFFFFF) << 8 * 3;
    uptime |= (searchMsg->struct1.uptime[2] & 0xFFFFFFFF) << 8 * 2;
    uptime |= (searchMsg->struct1.uptime[1] & 0xFFFFFFFF) << 8 * 1;
    uptime |= (searchMsg->struct1.uptime[0] & 0xFFFFFFFF);
    printf(" = %d\n", uptime);
    syslog(LOG_NOTICE, "## UP_TIME: %X-%X-%X-%x --- %d",
           searchMsg->struct1.uptime[3],
           searchMsg->struct1.uptime[1],
           searchMsg->struct1.uptime[2],
           searchMsg->struct1.uptime[0],
           uptime);
    printf("##   FIRMWARE VERSION: \t");
    printf(" %d %d %d\n", searchMsg->struct1.firmware[0], searchMsg->struct1.firmware[1],
           searchMsg->struct1.firmware[2]);
    syslog(LOG_NOTICE, "## FIRMWARE VERSION: %d %d %d\n",
           searchMsg->struct1.firmware[0],
           searchMsg->struct1.firmware[1],
           searchMsg->struct1.firmware[2]);

    printf("##  MASK: \t");
    for (int i = 0; i < IPV4_LEN; i++)
    {
        printf("%d", searchMsg->struct1.mask[i]);
        printf(" ");
    }
    printf("\n");

    syslog(LOG_NOTICE, "## MASK: %d.%d.%d.%d",
           searchMsg->struct1.mask[0],
           searchMsg->struct1.mask[1],
           searchMsg->struct1.mask[2],
           searchMsg->struct1.mask[3]);

    printf("##   GATEWAY: \t");
    for (int i = 0; i < IPV4_LEN; i++)
    {
        printf("%d", searchMsg->struct1.gate[i]);
        printf(" ");
    }
    printf("\n");
    syslog(LOG_NOTICE, "## GATEWAY: %d.%d.%d.%d",
           searchMsg->struct1.gate[0],
           searchMsg->struct1.gate[1],
           searchMsg->struct1.gate[2],
           searchMsg->struct1.gate[3]);
}

void getMainBuff(uint8_t *mainBuff)
{
    memcpy(mainBuff, searchOutMsg.buff, sizeof(searchOutMsg.buff));
}

void setMainTestFlag(uint8_t value)
{
    mainTestFlag = value;
}

