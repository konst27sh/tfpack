//
// Created by sheverdin on 6/25/24.
//

#include "port_module.h"
#include "utils_lib/utils_module.h"
#include "ping.h"

portInfo_t portInfoArr[NUM_PORTS];

static void port_timeAlarm(time_h_m *timeAlarmInfo);

error_code_t port_setLinkParam(port_config_info_t *portConfig)
{
    return ERR_OK;
}

error_code_t port_setHostParam(port_config_info_t *portConfig , char *host)
{
    int ipIsValid  = ERR_IP_NOT_VALID;
    strcpy(portConfig->host, host);
    checkValidIp(portConfig->host, &ipIsValid);
    return ipIsValid;
}

error_code_t port_setSpeedParam(port_config_info_t *portConfig, uint32_t max, uint32_t min)
{
    if (min > max)
    {
        return  ERR_SPEED_VALUE;
    }

    if (max < MIN_SPEED || max > MAX_SPEED)
    {
        return  ERR_SPEED_VALUE;
    }
    else
    {
        portConfig->max_speed = max;
    }

    if(min < MIN_SPEED || min > MAX_SPEED)
    {
        return ERR_SPEED_VALUE;
    }
    else
    {
        portConfig->min_speed = min;
    }
    return ERR_OK;
}

error_code_t port_setTimeParam(port_config_info_t *portConfig, char* timeUpStr, char *timeDownStr)
{
    struct tm timeInfo;
    struct tm *timeInfoPtr;
    time_t now = time(NULL);
    error_code_t errorCode = ERR_OK;
    errorCode |= isValidTime(timeUpStr, &timeInfo);

    if(errorCode == ERR_OK) {
        portConfig->timeAlarm[time_up].time_H = timeInfo.tm_hour;
        portConfig->timeAlarm[time_up].time_M = timeInfo.tm_min;

        timeInfoPtr = localtime(&now);
        timeInfoPtr->tm_hour   = portConfig->timeAlarm[time_up].time_H;
        timeInfoPtr->tm_min    = portConfig->timeAlarm[time_up].time_M;
        timeInfoPtr->tm_sec    = 0;
        portConfig->timeAlarm[time_up].targetTime = mktime(timeInfoPtr);

        if(now >= portConfig->timeAlarm[time_up].targetTime)
        {
            timeInfoPtr->tm_mday += 1;
            portConfig->timeAlarm[time_up].targetTime = mktime(timeInfoPtr);
        }
    }

    errorCode |= isValidTime(timeDownStr, &timeInfo);

    if(errorCode == ERR_OK) {
        portConfig->timeAlarm[time_down].time_H = timeInfo.tm_hour;
        portConfig->timeAlarm[time_down].time_M = timeInfo.tm_min;
        timeInfoPtr = localtime(&now);
        timeInfoPtr->tm_hour   = portConfig->timeAlarm[time_down].time_H;
        timeInfoPtr->tm_min    =  portConfig->timeAlarm[time_down].time_M;
        timeInfoPtr->tm_sec    = 0;
        portConfig->timeAlarm[time_down].targetTime = mktime(timeInfoPtr);

        if(now >= portConfig->timeAlarm[time_down].targetTime)
        {
            timeInfoPtr->tm_mday += 1;
            portConfig->timeAlarm[time_down].targetTime = mktime(timeInfoPtr);
        }
    }
    return errorCode;
}

portInfo_t *port_get_portInfoArr(void)
{
    return portInfoArr;
}

error_code_t port_set_PortReboot(port_reboot_info_t *portRebootInfo, uint8_t portNum)
{
    error_code_t errorCode = ERR_OK;
    void *res = memcpy(&portInfoArr[portNum].portRebootInfo, portRebootInfo, sizeof(port_reboot_info_t));
    if (res == NULL)
    {
        errorCode = ERR_NULL_OBJ;
    }

    return errorCode;
}

////////////////////////////////////////////////////////////////////
void port_runTestDisable(uint8_t portNum)
{
    uint8_t index = 0;
    if (portNum > 0 && portNum <= NUM_PORTS)
    {
        index = portNum - 1;
    }
    else
    {
        return;
    }
    portInfoArr[index].portResetInfo.errorCode &= (~ERR_OK);
    portInfoArr[index].portResetInfo.errorCode |= ERR_TEST_DISABLE;
}

void port_runTestLink(uint8_t portNum)
{
    uint8_t index = 0;
    if (portNum > 0 && portNum <= NUM_PORTS)
    {
        index = portNum - 1;
    }
    else
    {
        return;
    }

    char output[OUTPUT_MAX_LENGTH];
    char cmd_getOperstate[64];
    char portStr[2] = "";
    strcpy(output, "");
    strcpy(cmd_getOperstate, "");
    strcpy(portStr, "");

    FILE *portLink              = NULL;
    char str_up[]               = "up\n";
    char path_OperstatePrefix[] = "/sys/class/net/lan\0";
    char path_OperstateSufix[]  = "/operstate\0";

    toString(portNum, portStr);
    strcpy(cmd_getOperstate, path_OperstatePrefix);
    strcat(cmd_getOperstate, portStr);
    strcat(cmd_getOperstate, path_OperstateSufix);
    FILE *fp = fopen(cmd_getOperstate, "r");
    if (fp)
    {
        fgets(output, OUTPUT_MAX_LENGTH, fp);
    }
    else
    {
        openlog("tf_autoresart", 0, LOG_USER);
        syslog(LOG_ERR, "port <%d>: error open link status", portNum);
        closelog();
    }
    int fclose_flag = fclose(fp);
    if (fclose_flag)
    {
        openlog("tf_autoresart", 0, LOG_USER);
        syslog(LOG_ERR, "port <%d>: error close link status - %d", portNum, errno);
        closelog();
    }

    portInfoArr[index].portResetInfo.errorCode &= (~ERR_TEST_LINK);
    portInfoArr[index].portResetInfo.errorCode |= ERR_OK;

    if (strcmp(output, str_up)!=0) {
        portInfoArr[index].portResetInfo.errorCode &= (~ERR_OK);
        portInfoArr[index].portResetInfo.errorCode |= ERR_TEST_LINK;
    }
    else
    {
        if(portInfoArr[index].portConfigInfo.test == test_link)
        {
            if (portInfoArr[index].portResetInfo.state == IDLE_STATE)
            {
                portInfoArr[index].portResetInfo.event |= TEST_OK;
            }
        }
    }
}

void port_runTestPing(uint8_t portNum)
{
    uint8_t index = 0;
    if (portNum > 0 && portNum <= NUM_PORTS)
    {
        index = portNum - 1;
    }
    else
    {
        return;
    }

    if (portInfoArr[index].portResetInfo.errorCode & ERR_TEST_LINK)
    {
        return;
    }
    else
    {
        sendPingRequest(&portInfoArr[index].portConfigInfo);
    }
    if (portInfoArr[index].portConfigInfo.pingResult == ERR_TEST_PING)
    {
        portInfoArr[index].portResetInfo.errorCode &= (~ERR_OK);
        portInfoArr[index].portResetInfo.errorCode |= ERR_TEST_PING;
    }
    else if (portInfoArr[index].portConfigInfo.pingResult == ERR_OK)
    {
        portInfoArr[index].portResetInfo.errorCode &= (~ERR_TEST_PING);
        portInfoArr[index].portResetInfo.errorCode |= ERR_OK;

        if (portInfoArr[index].portResetInfo.state == IDLE_STATE)
        {
            portInfoArr[index].portResetInfo.event |= TEST_OK;
        }
    }
}

void port_runTestSpeed(uint8_t portNum)
{
    uint32_t delta_time     = 0;
    uint8_t index = 0;
    if (portNum > 0 && portNum <= NUM_PORTS)
    {
        index = portNum - 1;
    }
    else
    {
        return;
    }
    if (portInfoArr[index].portResetInfo.errorCode & ERR_TEST_LINK)
    {
        return;
    }
    else
    {
        char output[OUTPUT_MAX_LENGTH];
        char portStr[2];
        char cmd_get_rx_byte[128];
        char value_rxByte_str[32];
        uint8_t find_flag = 0;
        strcpy(output, "");
        strcpy(portStr, "");
        strcpy(cmd_get_rx_byte, "");
        strcpy(value_rxByte_str, "");

        FILE *pipe_portSpeed = NULL;
        char path_rx_bytePrefix[] = "ubus call net_stat getStatus '{\"port\":\"lan";
        char path_rx_byteSufix[] = "\", \"param\":\"ifInOctets\"}'";

        toString(portNum, portStr);
        strcpy(cmd_get_rx_byte, path_rx_bytePrefix);
        strcat(cmd_get_rx_byte, portStr);
        strcat(cmd_get_rx_byte, path_rx_byteSufix);

        pipe_portSpeed = openPipe(cmd_get_rx_byte);
        while (fgets(output, OUTPUT_MAX_LENGTH, pipe_portSpeed))
        {
            find_flag |= getValueFromJson(output, "nic_stat", value_rxByte_str);
        }
        closePipe(pipe_portSpeed, cmd_get_rx_byte);

        if (find_flag == 0)
        {
            portInfoArr[index].portResetInfo.errorCode &= (~ERR_OK);
            portInfoArr[index].portResetInfo.errorCode |= ERR_UNAVAILABLE_RESOURCE;
            return;
        }

        portInfoArr[index].portResetInfo.errorCode &= (~ERR_TEST_SPEED);
        portInfoArr[index].portResetInfo.errorCode |= ERR_OK;

        errno = 0;
        portInfoArr[index].portConfigInfo.rx_byte_current = strtoll(value_rxByte_str, NULL, 10);
        if (errno != 0)
        {
            perror("error strtol: ");
            portInfoArr[index].portResetInfo.errorCode &= (~ERR_OK);
            portInfoArr[index].portResetInfo.errorCode |= ERR_UNAVAILABLE_RESOURCE;
            return;
        }

        portInfoArr[index].portConfigInfo.rx_delta_byte = portInfoArr[index].portConfigInfo.rx_byte_current  - portInfoArr[index].portConfigInfo.rx_byte_prev;
        portInfoArr[index].portConfigInfo.rx_byte_prev = portInfoArr[index].portConfigInfo.rx_byte_current;
        portInfoArr[index].portConfigInfo.time_current = time(NULL);
        delta_time = portInfoArr[index].portConfigInfo.time_current - portInfoArr[index].portConfigInfo.time_prev;
    }

    portInfoArr[index].portConfigInfo.time_prev = portInfoArr[index].portConfigInfo.time_current;

    if (delta_time == 0) {
        portInfoArr[index].portResetInfo.errorCode &= (~ERR_OK);
        portInfoArr[index].portResetInfo.errorCode |= ERR_TEST_SPEED;
    }
    else
    {
        portInfoArr[index].portConfigInfo.rx_speed_Kbit = portInfoArr[index].portConfigInfo.rx_delta_byte / (delta_time);
        portInfoArr[index].portConfigInfo.rx_speed_Kbit =  portInfoArr[index].portConfigInfo.rx_speed_Kbit >> 7;

        if ((portInfoArr[index].portConfigInfo.rx_speed_Kbit  < portInfoArr[index].portConfigInfo.min_speed) ||
            (portInfoArr[index].portConfigInfo.rx_speed_Kbit  > portInfoArr[index].portConfigInfo.max_speed))
        {
            portInfoArr[index].portResetInfo.errorCode &= (~ERR_OK);
            portInfoArr[index].portResetInfo.errorCode |= ERR_TEST_SPEED;
        }
        else
        {
            if (portInfoArr[index].portResetInfo.state == IDLE_STATE)
            {
                portInfoArr[index].portResetInfo.event |= TEST_OK;
            }
        }
    }
}

void port_runTestTime(uint8_t portNum)
{
    uint8_t index = 0;
    if (portNum > 0 && portNum <= NUM_PORTS)
    {
        index = portNum - 1;
    }
    else
    {
        return;
    }
    port_timeAlarm(&portInfoArr[index].portConfigInfo.timeAlarm[time_up]);
    port_timeAlarm(&portInfoArr[index].portConfigInfo.timeAlarm[time_down]);
}

AR_STATE get_PortAndPoeState(uint8_t portNum)
{
    //printf("get_PortAndPoeState -- %d \n", portNum);
    uint8_t index = 0;
    if (portNum > 0 && portNum <= NUM_PORTS)
    {
        index = portNum - 1;
    }
    else
    {
        return IDLE_STATE;
    }

    AR_STATE  ar_state = REGULAR_STATE;
    char output[OUTPUT_MAX_LENGTH];
    char cmdPortState[128];
    char portStr[2];
    strcpy(output, "");
	strcpy(cmdPortState, "");
    strcpy(portStr, "");
    char s_enable[6]         = "enable";
    char str1[64]            = "ubus call uci get '{\"config\":\"port\", \"section\":\"lan";
    char str2[8]             =  "\"}'";
    toString(portNum, portStr);

    strcpy(cmdPortState, str1);
    strcat(cmdPortState, portStr);
    strcat(cmdPortState, str2);

    FILE *pipePortState = openPipe(cmdPortState);
    char  res[256];
    strcpy(res, "");
    while (fgets(output, OUTPUT_MAX_LENGTH, pipePortState))
    {
        strcat(res, output);
    }
    closePipe(pipePortState, cmdPortState);

    char state_str[32];
    char poe_str[32];
	strcpy(state_str, "");
	strcpy(poe_str, "");
    getData_formJson(res, "state", state_str);
    getData_formJson(res, "poe", poe_str);

    if (strcmp (state_str, s_enable)==0)
    {
        portInfoArr[index].portStateInfo.port_state = enable;
    }
    else
    {
        portInfoArr[index].portStateInfo.port_state = disable;
        portInfoArr[index].portResetInfo.errorCode &= (~ERR_OK);
        portInfoArr[index].portResetInfo.errorCode |= ERR_PORT_DISABLE;
        portInfoArr[index].portResetInfo.state = IDLE_STATE;
        ar_state = portInfoArr[index].portResetInfo.state;
    }

    if (strcmp (poe_str, s_enable)==0)
    {
        portInfoArr[index].portStateInfo.poe_state = enable;
    }
    else
    {
        portInfoArr[index].portStateInfo.poe_state = disable;
        portInfoArr[index].portResetInfo.errorCode &= (~ERR_OK);
        portInfoArr[index].portResetInfo.errorCode |= ERR_PoE_DISABLE;
        portInfoArr[index].portResetInfo.state = IDLE_STATE;
        ar_state = portInfoArr[index].portResetInfo.state;
    }
    return ar_state;
}

static void port_timeAlarm(time_h_m *timeAlarmInfo)
{
    time_t now = time(NULL);
    struct tm *timeInfo = localtime(&now);
    timeInfo->tm_hour   = timeAlarmInfo->time_H;
    timeInfo->tm_min    = timeAlarmInfo->time_M;
    timeInfo->tm_sec    = 0;
    now = time(NULL);
    if (now >= timeAlarmInfo->targetTime) {
        timeInfo = localtime(&now);
        timeInfo->tm_mday += 1; // add one day
        timeAlarmInfo->targetTime = mktime(timeInfo);
        timeAlarmInfo->remainTime = 0;
        timeAlarmInfo->status = enable;
    }
    else
    {
        timeAlarmInfo->remainTime = timeAlarmInfo->targetTime - now;
    }
}

void poe_Control(uint8_t port, POE_CONTROL poeControl)
{
    FILE *pipe_poeRestart = NULL;
    char output[OUTPUT_MAX_LENGTH];
    char cmdPortRestart[64];
    char portStr[2];
    strcpy(output, "");
    strcpy(cmdPortRestart, "");
    strcpy(portStr, "");

    char str1[]             = "ubus call poe manage '{\"port\":\"lan"; //   1\", \"enable\":true}'";
    char strTrue[]          = "\", \"enable\":true}'";
    char strFalse[]         = "\", \"enable\":false}'";

    toString(port, portStr);
    if(poeControl & POE_DOWN)
    {
        strcpy(cmdPortRestart, str1);
        strcat(cmdPortRestart, portStr);
        strcat(cmdPortRestart, strFalse);
        pipe_poeRestart = openPipe(cmdPortRestart);
        closePipe(pipe_poeRestart, cmdPortRestart);
        openlog("tf_autoresart", 0, LOG_USER);
        syslog(LOG_INFO, "port <%d>: PoE down", port);
        closelog();
    }

    if(poeControl & POE_UP)
    {
        strcpy(cmdPortRestart, "");
        strcpy(cmdPortRestart, str1);
        strcat(cmdPortRestart, portStr);
        strcat(cmdPortRestart, strTrue);
        pipe_poeRestart = openPipe(cmdPortRestart);
        closePipe(pipe_poeRestart, cmdPortRestart);
        openlog("tf_autoresart", 0, LOG_USER);
        syslog(LOG_INFO, "port <%d>: PoE up", port);
        closelog();
    }
}

void autoResetHandler(uint8_t portNum, uint8_t maxReset)
{
    uint8_t index = 0;
    if (portNum > 0 && portNum <= NUM_PORTS)
    {
        index = portNum - 1;
    }
    else
    {
        return;
    }

    char  timeDateStr[32];
    strcpy(timeDateStr, "");
    getTimeDate(timeDateStr);
    strcpy(portInfoArr[index].portConfigInfo.timeStr, "");
    strcpy(portInfoArr[index].portConfigInfo.timeStr, timeDateStr);
    portInfoArr[index].portResetInfo.errorCode &= (~ERR_OK);

    if (portInfoArr[index].portResetInfo.resetCount >= maxReset)
    {
        portInfoArr[index].portResetInfo.errorCode &= (~ERR_REBOOTING);
        portInfoArr[index].portResetInfo.errorCode |= ERR_PORT_SHUTDOWN;
        portInfoArr[index].portRebootInfo.rebootState =  REBOOT_STATE_IDLE;
        portInfoArr[index].portResetInfo.state = IDLE_STATE;
        syslog(LOG_ALERT, "port <%d>: reboot disable after max reset", portNum);
        closelog();
        set_errorHandler(index);
    }
    else if (portInfoArr[index].portResetInfo.resetCount < maxReset)
    {

        char errorMsg[256];
        uint8_t error_flag = 0;
        strcpy(errorMsg, "error test: ");
        if(portInfoArr[index].portResetInfo.errorCode & ERR_TEST_LINK)
        {
            error_flag = 1;
            strcat(errorMsg, "link");
        }

        if(portInfoArr[index].portResetInfo.errorCode & ERR_TEST_SPEED)
        {
            if (error_flag)
                strcat(errorMsg, ", ");
            strcat(errorMsg, "speed");
        }

        if(portInfoArr[index].portResetInfo.errorCode & ERR_TEST_PING)
        {
            if (error_flag)
                strcat(errorMsg, ", ");
            strcat(errorMsg, "ping");
        }

        openlog("tf_autorestart", 0, LOG_USER);
        syslog(LOG_ALERT, "port <%d>: auto reboot, total reset = %d,  %s", portNum, portInfoArr[index].portResetInfo.totalResetCount, errorMsg);
        closelog();
        portInfoArr[index].portResetInfo.errorTestCount = 0;
        portInfoArr[index].portResetInfo.resetCount++;
        portInfoArr[index].portResetInfo.totalResetCount++;
        portInfoArr[index].portResetInfo.errorCode |= ERR_REBOOTING;
        portInfoArr[index].portRebootInfo.rebootState = REBOOT_STATE_INIT;
    }
}

void manualResetHandler(uint8_t portNum)
{
    uint8_t index = 0;
    if (portNum > 0 && portNum <= NUM_PORTS)
    {
        index = portNum - 1;
    }
    else
    {
        return;
    }

    char  timeDateStr[32];
    strcpy(timeDateStr, "");
    getTimeDate(timeDateStr);

    strcpy(portInfoArr[index].portConfigInfo.timeStr, "");
    strcpy(portInfoArr[index].portConfigInfo.timeStr, timeDateStr);

    portInfoArr[index].portResetInfo.errorCode &= (~ERR_OK);
    portInfoArr[index].portResetInfo.errorCode &= (~ERR_TEST_LINK);
    portInfoArr[index].portResetInfo.errorCode &= (~ERR_TEST_PING);
    portInfoArr[index].portResetInfo.errorCode &= (~ERR_TEST_SPEED);
    portInfoArr[index].portResetInfo.errorCode &= (~ERR_PORT_SHUTDOWN);

    portInfoArr[index].portResetInfo.errorCode |= ERR_MANUAL_REBOOT;
    portInfoArr[index].portResetInfo.totalResetCount++;
    portInfoArr[index].portResetInfo.resetCount     = 0;
    portInfoArr[index].portResetInfo.errorTestCount = 0;
    portInfoArr[index].portRebootInfo.rebootState = REBOOT_STATE_INIT;
}

void set_errorHandler(uint8_t index)
{
    if (
        portInfoArr[index].portResetInfo.errorCode      & ERR_IP_NOT_VALID
        || portInfoArr[index].portResetInfo.errorCode   & ERR_TIME_NOT_VALID
        || portInfoArr[index].portResetInfo.errorCode   & ERR_SPEED_VALUE
        || portInfoArr[index].portResetInfo.errorCode   & ERR_PORT_DISABLE
        )
    {
        portInfoArr[index].portConfigInfo.test = test_disable;
    }
}





