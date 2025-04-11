//
// Created by BelyaevAlex on 18.12.2024.
//

#ifndef TFORTIS_PACKAGES_MAIN_H
#define TFORTIS_PACKAGES_MAIN_H

#define ULOG_DBG(fmt, ...) ulog(LOG_DEBUG, fmt, ## __VA_ARGS__)

/* Careful with this; Only works for set_detection/disconnect_type commands. */
#define PORT_ID_ALL	0x7f
#define MAX_PORT	24
#define MAX_RETRIES	5
#define GET_STR(a, b)	((a) < ARRAY_SIZE(b) ? (b)[a] : NULL)
#define MAX(a, b)	(((a) > (b)) ? (a) : (b))


#define SET_PORT_ENABLE_CMD         0x01// 	Set port enable
#define SET_POWER_BUDGET_CMD        0x04//    Set global power budget
#define SET_PORT_MAP_ENABLE_CMD     0x05// 	Set port map enable - с этим перестаёт работать
#define SET_PORT_DET_TYPE_CMD       0x09//    Set port detection type
#define SET_DEV_CFG_CMD             0x0E//	Set device config - с этим перестаёт работать
#define SET_PORT_DISC_TYPE_CMD      0x0F//	Set port Disconnect Type
#define SET_PORT_MNGT_MODE_CMD      0x10//	Set power management mode
//#define SET_CLS_ENABLE_CMD        0x11//	Set classification enable
#define SET_PORT_PWR_LIMIT_CMD      0x12//	Set port power limit type
#define SET_PORT_PWR_BUDGET_CMD     0x13// 	Set port power budget
#define SET_PORT_PRIORITY_CMD       0x15//	Set port priority

#define GET_SYSTEM_INFO_CMD         0x40// 	Get system info
#define GET_GLOBAL_PWR_STATUS_CMD   0x41//  Get global power status
#define GET_PORT_STATUS_CMD         0x42//	Get port status
#define GET_GROUP_PORT_STATUS_CMD   0x43//	Get group port status
#define GET_PORT_MEAS_CMD           0x44// 	Get port measurements
#define GET_PORT_COUNTERS_CMD       0x45// 	Get port Mib counters
#define GET_PORT_CFG_CMD            0x48//  Get port basic configuration
#define GET_EXT_PORT_CFG_CMD        0x49// 	Get extended port config
#define GET_CHANNEL_STATUS_CMD      0x4E// 	Get channel status
#define GET_CHANNEL_POWER_CMD       0x4F// 	Get channel current and voltage
#define GET_PORT_OVERVIEW_CMD       0x50// 	Get port overview


#define STAT_DISABLED               0//Disabled
#define STAT_SEARCHING              1//Searching
#define STAT_DELIVERING             2//Delivering power
#define STAT_FAULT                  4//Fault
#define STAT_OTHER_FAULT            5//Other Fault
#define STAT_REQUESTING             6//Requesting power

#define CONN_2PAIR                  0
#define CONN_SINGLE_PD              1
#define CONN_DUAL_PD                2

struct port_config {
    char name[16];
    unsigned int valid : 1;
    unsigned int enable : 1;
    unsigned char priority;
    unsigned char power_up_mode;
    unsigned char power_budget;
};

struct config {
    float budget;
    float budget_guard;

    unsigned int port_count;
    uint8_t pse_id_set_budget_mask;
    struct port_config ports[MAX_PORT];
};

struct port_state {
    const char *status_str; //text status
    unsigned char status;//power status
    unsigned char faults;//fault or error types
    const char *faults_str;
    unsigned char det_status;//detection status
    const char *det_status_str;
    unsigned char cls_status;//classification status
    const char *cls_status_str;
    unsigned char conn_status;//connection status
    const char *conn_status_str;
    unsigned char status_old;
    float power;//port power in Watt
    float voltage;//Voltage in Volts
    unsigned int current;//Current in mA
    float temperature;
    const char *poe_mode;

    const char *status_a_str; //text status
    unsigned char status_a;//power status
    const char *status_b_str; //text status
    unsigned char status_b;//power status
    unsigned char faults_a;//fault or error types
    const char *faults_a_str;
    unsigned char faults_b;//fault or error types
    const char *faults_b_str;
    unsigned char det_status_a;//detection status
    const char *det_status_a_str;
    unsigned char det_status_b;//detection status
    const char *det_status_b_str;
    unsigned char cls_status_a;
    const char *cls_status_a_str;
    unsigned char cls_status_b;
    const char *cls_status_b_str;

    float voltage_a;//Voltage in Volts
    float voltage_b;//Voltage in Volts
    unsigned int current_a;//Current in mA
    unsigned int current_b;//Current in mA
};

struct state {
    const char *sys_mode;
    unsigned char sys_version;
    const char *sys_mcu;
    const char *sys_status;
    unsigned char sys_ext_version;
    float power_consumption;
    unsigned int num_detected_ports;

    struct port_state ports[MAX_PORT];
    struct uloop_timeout mcu_error_timeout;
};

struct cmd {
    struct list_head list;
    unsigned char cmd[12];
    unsigned int num_retries;
};



int poe_cmd_queue(unsigned char *_cmd, int len);
int poe_port_setup(void);
int poe_cmd_port_enable(unsigned char port, unsigned char enable);

#endif //TFORTIS_PACKAGES_MAIN_H
