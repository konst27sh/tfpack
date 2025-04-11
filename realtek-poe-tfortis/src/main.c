/* SPDX-License-Identifier: GPL-2.0 */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

#include <libubox/ustream.h>
#include <libubox/uloop.h>
#include <libubox/list.h>
#include <libubox/ulog.h>
#include <libubus.h>

#include <uci.h>
#include <uci_blob.h>

#include <syslog.h>

#include "ubus.h"
#include "config.h"
#include "main.h"


typedef int (*poe_reply_handler)(unsigned char *reply);

static struct ustream_fd stream;
static LIST_HEAD(cmd_pending);
static unsigned char cmd_seq;
struct state state;

struct config config = {
	.budget = 65,
	.budget_guard = 7,
	.pse_id_set_budget_mask = 0x01,
};


static uint16_t read16_be(uint8_t *raw)
{
	return (uint16_t)raw[0] << 8 | raw[1];
}

static void write16_be(uint8_t *raw, uint16_t value)
{
	raw[0] = value >> 8;
	raw[1] =  value & 0xff;
}


static void log_packet(int log_level, const char *prefix, const uint8_t d[12])
{
	ulog(log_level,
	     "%s %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
	     prefix, d[0], d[1], d[2], d[3], d[4], d[5],
		     d[6], d[7], d[8], d[9], d[10], d[11]);
}

static int poe_cmd_send(struct cmd *cmd)
{
	if (state.mcu_error_timeout.pending){
		ulog(LOG_DEBUG, "poe_cmd_send EBUSY\n");
		return -EBUSY;
	}
	log_packet(LOG_DEBUG, "TX ->", cmd->cmd);
	ustream_write(&stream.stream, (char *)cmd->cmd, 12, false);
	return 0;
}

static int poe_cmd_next(void)
{
	struct cmd *cmd;

	if (list_empty(&cmd_pending))
		return -1;

	cmd = list_first_entry(&cmd_pending, struct cmd, list);

	return poe_cmd_send(cmd);
}

int poe_cmd_queue(unsigned char *_cmd, int len)
{
	int i;
	int empty = list_empty(&cmd_pending);
	struct cmd *cmd = malloc(sizeof(*cmd));

	memset(cmd, 0, sizeof(*cmd));
	memset(cmd->cmd, 0xff, 12);
	memcpy(cmd->cmd, _cmd, len);

	cmd_seq++;

	cmd->cmd[1] = cmd_seq;
	cmd->cmd[11] = 0;

	for (i = 0; i < 11; i++)
		cmd->cmd[11] += cmd->cmd[i];

	list_add_tail(&cmd->list, &cmd_pending);

	if (empty){
		ulog(LOG_DEBUG, "poe_cmd_queue empty\n");
		return poe_cmd_send(cmd);
	}

	return 0;
}

static int poet_cmd_4_port(uint8_t cmd_id, uint8_t port[4], uint8_t data[4])
{
	uint8_t cmd[] = { cmd_id, 0x00, port[0], data[0], port[1], data[1],
					port[2], data[2], port[3], data[3] };

	return poe_cmd_queue(cmd, sizeof(cmd));
}

/* 0x01 - Set port enable
 *	0: Disable
 *	1: Enable
 */
int poe_cmd_port_enable(unsigned char port, unsigned char enable)
{
	unsigned char cmd[] = { 0x01, 0x00, port, enable };

	return poe_cmd_queue(cmd, sizeof(cmd));
}

static int poe_cmd_port_mapping_enable(bool enable)
{
	unsigned char cmd[] = { 0x02, 0x00, enable };

	return poe_cmd_queue(cmd, sizeof(cmd));
}

/* 0x09 - Set port detection type
 *	1: Legacy Capacitive Detection only
 *	2: IEEE 802.3af 4-Point Detection only (Default)
 *	3: IEEE 802.3af 4-Point followed by Legacy
 *	4: IEEE 802.3af 2-Point detection (Not Supported)
 *	5: IEEE 802.3af 2-Point followed by Legacy
 */
static int poe_cmd_port_detection_type(unsigned char port, unsigned char type)
{
	unsigned char cmd[] = { 0x09, 0x00, port, type };

	return poe_cmd_queue(cmd, sizeof(cmd));
}

/* 0x11 - Set port classification
 *	0: Disable
 *	1: Enable
 */
static int poe_cmd_port_classification(uint8_t port[4], uint8_t enable[4])
{
	return poet_cmd_4_port(0x11, port, enable);
}

/* 0x0F - Set port disconnect type
 *	0: none
 *	1: AC-disconnect
 *	2: DC-disconnect
 *	3: DC with delay
 */
static int poe_cmd_port_disconnect_type(unsigned char port, unsigned char type)
{
	unsigned char cmd[] = { 0x0F, 0x00, port, type };

	return poe_cmd_queue(cmd, sizeof(cmd));
}

/* 0x12 - Set port power limit type
 *	0: None. Power limit is 16.2W if the connected device is “low power”,
 *	   or the set high power limit if the device is “high power”.
 *	1: Class based. The power limit for class 4 devices is determined by the high power limit.
 *	2: User defined
 */
static int poe_cmd_port_power_limit_type(uint8_t port[4], uint8_t limit[4])
{
	return poet_cmd_4_port(0x12, port, limit);
}

/* 0x13 - Set port power budget
 *	values in 0.2W increments
 */
static int poe_cmd_port_power_budget(unsigned char port, unsigned char budget)
{
	unsigned char cmd[] = { 0x13, 0x00, port, budget };

	return poe_cmd_queue(cmd, sizeof(cmd));
}

/* 0x10 - Set power management mode
 *	0: None (No Power Management mode) (Default in Semi-Auto mode)
 *	1: Static Power Management with Port Priority(Default in Automode)
 *	2: Dynamic Power Management with Port Priority
 *	3: Static Power Management without Port Priority
 *	4: Dynamic Power Management without Port Priority
 */
static int poe_cmd_power_mgmt_mode(unsigned char mode)
{
	unsigned char cmd[] = { 0x10, 0x00, mode };//todo

	return poe_cmd_queue(cmd, sizeof(cmd));
}

/* 0x04 - Set global power budget */
static int poe_cmd_global_power_budget(uint8_t pse, float budget, float guard)
{
	uint8_t cmd[] = { 0x18, 0x00, pse, 0x00, 0x00, 0x00, 0x00 };

	write16_be(cmd + 3, budget * 10);
	write16_be(cmd + 5, guard * 10);

	return poe_cmd_queue(cmd, sizeof(cmd));
}

/* 0x15 - Set port priority
 *	0: Low
 *	1: Normal
 *	2: High
 *	3: Critical
 */
static int poe_set_port_priority(uint8_t port[4], uint8_t priority[4])
{
	return poet_cmd_4_port(0x15, port, priority);
}

/* 0x1c - Set port power-up mode
 *	0: PoE
 *	1: legacy
 *	2: pre-PoE+
 *	3: PoE+
 */
static int poe_set_port_power_up_mode(uint8_t port[4], uint8_t mode[4])
{
	return poet_cmd_4_port(0x1c, port, mode);
}

/* 0x40 - Get system info */
static int poe_cmd_status(void)
{
	unsigned char cmd[] = { GET_SYSTEM_INFO_CMD };

	return poe_cmd_queue(cmd, sizeof(cmd));
}

static int poe_reply_status(unsigned char *reply)
{
	const char *mode[] = {
		"Semi-auto I2C",
		"Semi-auto UART",
		"Auto I2C",
		"Auto UART"
	};
	const char *mcu[] = {
		"GigaDevice GD32F310",
		"GigaDevice GD32E230",
		"GigaDevice GD32F303",
		"GigaDevice GD32E103"		
	};
	const char *status[] = {
		"Global Disable pin is de-asserted:No system reset from the previous query cmd:Configuration saved",
		"Global Disable pin is de-asserted:No system reset from the previous query cmd:Configuration Dirty",
		"Global Disable pin is de-asserted:System reseted:Configuration saved",
		"Global Disable pin is de-asserted:System reseted:Configuration Dirty",
		"Global Disable Pin is asserted:No system reset from the previous query cmd:Configuration saved",
		"Global Disable Pin is asserted:No system reset from the previous query cmd:Configuration Dirty",
		"Global Disable Pin is asserted:System reseted:Configuration saved",
		"Global Disable Pin is asserted:System reseted:Configuration Dirty"
	};

	state.sys_mode = GET_STR(reply[2], mode);
	state.num_detected_ports = reply[3];
	state.sys_version = reply[7];
	state.sys_mcu = GET_STR(reply[8], mcu);
	state.sys_status = GET_STR(reply[9], status);
	state.sys_ext_version = reply[10];

	return 0;
}

/* 0x41 - Get power statistics */
static int poe_cmd_power_stats(void)
{
	unsigned char cmd[] = { GET_GLOBAL_PWR_STATUS_CMD };

	return poe_cmd_queue(cmd, sizeof(cmd));
}

static int poe_reply_power_stats(unsigned char *reply)
{
	state.power_consumption = read16_be(reply + 7) * 0.1;

	return 0;
}

/* 0x48 - Get basic port config */
static int poe_cmd_port_config(unsigned char port)
{
	unsigned char cmd[] = { GET_PORT_CFG_CMD, 0x00, port };

	return poe_cmd_queue(cmd, sizeof(cmd));
}

/* 0x49 - Get extended port config */
static int poe_cmd_port_ext_config(unsigned char port)
{
	unsigned char cmd[] = { GET_EXT_PORT_CFG_CMD, 0x00, port };

	return poe_cmd_queue(cmd, sizeof(cmd));
}

static int poe_reply_port_ext_config(unsigned char *reply)
{
	const char *mode[] = {
		"PoE",
		"Legacy",
		"pre-PoE+",
		"PoE+"
	};

	state.ports[reply[2]].poe_mode = GET_STR(reply[3], mode);

	return 0;
}

static int poe_reply_port_config(unsigned char *reply){

	log_packet(LOG_DEBUG, "---poe_reply_port_config <-", reply);

	return 0;
}

/* 0x43 - Get port status of group 
	group 0 - ports 0..3
	group 1 - ports 4..7, etc	
*/
const char *port_status[] = {
        [STAT_DISABLED] = "Disabled",
        [STAT_SEARCHING] = "Searching",
        [STAT_DELIVERING] = "Delivering power",
        [3] = "rsvd",
        [STAT_FAULT] = "Fault",
        [5] = "Other fault",
        [STAT_REQUESTING] = "Requesting power",
};


static int poe_cmd_4_port_status(uint8_t group)
{
	uint8_t cmd[] = { GET_GROUP_PORT_STATUS_CMD, 0x00, group};

	return poe_cmd_queue(cmd, sizeof(cmd));
}

static int poe_reply_4_port_status(uint8_t *reply)
{
	int i, port, pstate;

	for (i = 0; i < 4; i++) {
		port = reply[2]*4+i;
		pstate = reply[3+i*2];

		if (port == 0xff) {
			continue;
		} else if (port >= MAX_PORT) {
			ULOG_WARN("Invalid port status packet (port=%d)\n", port);
			return -1;
		}

		state.ports[port].status_str = GET_STR(pstate & 0xf, port_status);
		state.ports[port].status = (pstate & 0xf);
	}
	return 0;
}

/* 0x44 - Get port power statistics */
static int poe_cmd_port_power_stats(unsigned char port)
{
	unsigned char cmd[] = { GET_PORT_MEAS_CMD, 0x00, port };

	return poe_cmd_queue(cmd, sizeof(cmd));
}

static int poe_reply_port_power_stats(unsigned char *reply)
{
	int port_idx = reply[2];
	state.ports[port_idx].voltage = read16_be(reply + 3)*0.06545;
	state.ports[port_idx].current = read16_be(reply + 5);
	state.ports[port_idx].temperature = (read16_be(reply + 7) - 120) * (-1.25) + 125;
	state.ports[port_idx].power = read16_be(reply + 9) * 0.1;
	return 0;
}


/* 0x42 - Get port statistics */
static int poe_cmd_poe_status(unsigned char port){
    unsigned char cmd[] = { GET_PORT_STATUS_CMD, 0x00, port };

    return poe_cmd_queue(cmd, sizeof(cmd));
}
const char *det_status[] = {
        [0] = "Unknown",
        [1] = "Short current",
        [2] = "High Cap",
        [3] = "Rlow",
        [4] = "Valid PD",
        [5] = "Rhigh",
        [6] = "Open Circuit",
        [7] = "FET Fail",
        [8] = "reserved",
};

const char *cls_status[] = {
        [0] = "Class 0",
        [1] = "Class 1",
        [2] = "Class 2",
        [3] = "Class 3",
        [4] = "Class 4",
        [5] = "Class 5",
        [6] = "Class 6",
        [7] = "Class 7",
        [15] = "Class Mismatch",
        [16] = "Class over Current"
};
const char *con_status[] = {
        [0] = "2pair",
        [1] = "Single PD",
        [2] = "Dual PD",

};

const char *err_status[] = {
        [0] = "OVLO",
        [1] = "MPS Absent",
        [2] = "Short",
        [3] = "Overload",
        [4] = "Power Denied",
        [5] = "Thermal Shutdown",
        [7] = "UVLO",
};
static int poe_reply_port_stats(unsigned char *reply)
{
    int port_idx = reply[2];
    int pstatus = reply[3];
    state.ports[port_idx].status_str = GET_STR(pstatus & 0xf, port_status);
    state.ports[port_idx].status = (pstatus & 0xf);

    if(pstatus == STAT_DELIVERING){
        state.ports[port_idx].det_status = reply[4] & 0x0F;
        state.ports[port_idx].det_status_str = GET_STR(state.ports[port_idx].det_status, det_status);

        state.ports[port_idx].cls_status = (reply[4]>>4) & 0x0F;
        state.ports[port_idx].cls_status_str = GET_STR(state.ports[port_idx].cls_status, cls_status);
    }
    else if(pstatus == STAT_FAULT || pstatus == STAT_OTHER_FAULT){
        state.ports[port_idx].faults = reply[4];
        state.ports[port_idx].faults_str = GET_STR(state.ports[port_idx].faults, err_status);
    }


    state.ports[port_idx].conn_status = reply[7];
    state.ports[port_idx].conn_status_str = GET_STR(state.ports[port_idx].conn_status, con_status);

    if(state.ports[port_idx].conn_status ==CONN_DUAL_PD){
        state.ports[port_idx].cls_status_a = reply[5] & 0x0F;
        state.ports[port_idx].cls_status_a_str = GET_STR(state.ports[port_idx].cls_status_a, cls_status);
        state.ports[port_idx].cls_status_b = (reply[5]>>4) & 0x0F;
        state.ports[port_idx].cls_status_b_str = GET_STR(state.ports[port_idx].cls_status_b, cls_status);
        state.ports[port_idx].cls_status_b_str = GET_STR(state.ports[port_idx].cls_status_b, cls_status);
    }
    return 0;
}

/* 0x42 - Get port statistics */
static int poe_cmd_channel_status(unsigned char port){
    unsigned char cmd[] = { GET_CHANNEL_STATUS_CMD, 0x00, port };

    return poe_cmd_queue(cmd, sizeof(cmd));
}
static int poe_reply_channel_stats(unsigned char *reply){
    int port_idx = reply[2];
    int status_a = reply[6];

    //channel A
    state.ports[port_idx].status_a = (reply[6] & 0xf);
    state.ports[port_idx].status_a_str = GET_STR(state.ports[port_idx].status_a & 0xf, port_status);

    state.ports[port_idx].det_status_a = reply[3] & 0x0F;
    state.ports[port_idx].det_status_a_str = GET_STR(state.ports[port_idx].det_status_a, det_status);

    if(state.ports[port_idx].status_a == STAT_DELIVERING){
        state.ports[port_idx].cls_status_a = reply[4] & 0x0F;
        state.ports[port_idx].cls_status_a_str = GET_STR(state.ports[port_idx].cls_status_a, cls_status);
    }
    else if(state.ports[port_idx].status_a == STAT_FAULT || state.ports[port_idx].status_a == STAT_OTHER_FAULT){
        state.ports[port_idx].faults_a = reply[5];
        state.ports[port_idx].faults_a_str = GET_STR(state.ports[port_idx].faults_a, err_status);
    }

    //channel B
    state.ports[port_idx].status_b = (reply[10] & 0xf);
    state.ports[port_idx].status_b_str = GET_STR(state.ports[port_idx].status_b & 0xf, port_status);
    state.ports[port_idx].det_status_b = reply[7] & 0x0F;
    state.ports[port_idx].det_status_b_str = GET_STR(state.ports[port_idx].det_status_b, det_status);
    if(state.ports[port_idx].status_b == STAT_DELIVERING){
        state.ports[port_idx].cls_status_b = reply[8] & 0x0F;
        state.ports[port_idx].cls_status_b_str = GET_STR(state.ports[port_idx].cls_status_b, cls_status);
    }
    else if(state.ports[port_idx].status_b == STAT_FAULT || state.ports[port_idx].status_b == STAT_OTHER_FAULT){
        state.ports[port_idx].faults_b = reply[9];
        state.ports[port_idx].faults_b_str = GET_STR(state.ports[port_idx].faults_b, err_status);
    }
    return 0;
}


static int poe_cmd_channel_pwr_status(unsigned char port){
    unsigned char cmd[] = { GET_CHANNEL_POWER_CMD, 0x00, port };

    return poe_cmd_queue(cmd, sizeof(cmd));
}
static int poe_reply_channel_pwr_stats(unsigned char *reply) {
    int port_idx = reply[2];

    state.ports[port_idx].voltage_a = read16_be(reply + 3)*0.06545;
    state.ports[port_idx].current_a = read16_be(reply + 5);

    state.ports[port_idx].voltage_b = read16_be(reply + 7)*0.06545;
    state.ports[port_idx].current_b = read16_be(reply + 9);
    return 0;
}


static poe_reply_handler reply_handler[] = {
	[GET_SYSTEM_INFO_CMD] = poe_reply_status,
	[GET_GLOBAL_PWR_STATUS_CMD] = poe_reply_power_stats,
	[GET_PORT_CFG_CMD] = poe_reply_port_config,
	[GET_EXT_PORT_CFG_CMD] = poe_reply_port_ext_config,
	//[GET_GROUP_PORT_STATUS_CMD] = poe_reply_4_port_status,
	[GET_PORT_MEAS_CMD] = poe_reply_port_power_stats,
    [GET_PORT_STATUS_CMD] = poe_reply_port_stats,
    [GET_CHANNEL_STATUS_CMD] = poe_reply_channel_stats,
    [GET_CHANNEL_POWER_CMD] = poe_reply_channel_pwr_stats
};

static void mcu_clear_timeout(struct uloop_timeout *t)
{
	poe_cmd_next();
}

static void handle_poe_f0_reply(struct cmd *cmd, uint8_t *reply)
{
	const char *reason;

	const char *reasons[] = {
		[0xd] = "request-incomplete",
		[0xe] = "request-bad-checksum",
		[0xf] = "not-ready",
	};

	reason = GET_STR((uint8_t)(reply[0] - 0xf0), reasons);
	reason = reason ? reason : "unknown";

	/* Log the first reply, then only log complete failures. */
	if (cmd->num_retries == 0) {
		ULOG_NOTE("MCU rejected command: %s\n", reason);
		log_packet(LOG_NOTICE, "\tCMD:   ", cmd->cmd);
		log_packet(LOG_NOTICE, "\treply: ", reply);
	}

	if (!state.mcu_error_timeout.pending) {
		if (++cmd->num_retries > MAX_RETRIES) {
			ULOG_ERR("Aborting request (%02x) after %d attempts\n",
				 cmd->cmd[0], cmd->num_retries);
			free(cmd);
			return;
		}

		/* Wait for the MCU to recover */
		state.mcu_error_timeout.cb = mcu_clear_timeout;
		uloop_timeout_set(&state.mcu_error_timeout, 100);
	}

	list_add(&cmd->list, &cmd_pending);
}

static int poe_reply_consume(unsigned char *reply)
{
	struct cmd *cmd = NULL;
	unsigned char sum = 0, i;
	uint8_t cmd_id, cmd_seq;

	log_packet(LOG_DEBUG, "RX <-", reply);

	if (list_empty(&cmd_pending)) {
		ULOG_ERR("received unsolicited reply\n");
		return -1;
	}

	cmd = list_first_entry(&cmd_pending, struct cmd, list);
	list_del(&cmd->list);
	cmd_id = cmd->cmd[0];
	cmd_seq = cmd->cmd[1];

	for (i = 0; i < 11; i++)
		sum += reply[i];

	if (reply[11] != sum) {
		ULOG_DBG("received reply with bad checksum\n");
		free(cmd);
		return -1;
	}

	if ((reply[0] & 0xf0) == 0xf0) {
		handle_poe_f0_reply(cmd, reply);
		return -1;
	}

	free(cmd);

	if ((reply[0] != cmd_id) || (reply[0] > ARRAY_SIZE(reply_handler))) {
		ULOG_DBG("received reply with bad command id\n");
		return -1;
	}

	if (reply[1] != cmd_seq) {
		ULOG_DBG("received reply with bad sequence number\n");
		return -1;
	}

	if (reply_handler[reply[0]]) {
	  return reply_handler[reply[0]](reply);
	}

	return 0;
}

static void poe_stream_msg_cb(struct ustream *s, int bytes)
{
	int len;
	unsigned char *reply = (unsigned char *)ustream_get_read_buf(s, &len);

	ULOG_DBG("poe_stream_msg_cb\n");

	if (len < 12){
		return;
	}

	poe_reply_consume(reply);
	ustream_consume(s, 12);
	poe_cmd_next();
}

static void poe_stream_notify_cb(struct ustream *s)
{
	

	if (!s->eof){
		ULOG_DBG("poe_stream_notify_cb !s->eof\n");
		return;
	}

	ULOG_ERR("tty error, shutting down\n");
	exit(-1);
}

static int poe_stream_open(char *dev, struct ustream_fd *s, speed_t speed)
{
	int ret, tty;

	struct termios tio = {
		.c_oflag = 0,
		.c_iflag = 0,
		.c_cflag = speed | CS8 | CREAD | CLOCAL,
		.c_lflag = 0,
		.c_cc = {
			[VMIN] = 1,
		}
	};

	tty = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (tty < 0) {
		ULOG_ERR("%s: device open failed: %s\n", dev, strerror(errno));
		return -1;
	}

	ret = tcsetattr(tty, TCSANOW, &tio);
	if (ret) {
		ULOG_ERR("Can't configure serial port: %s", strerror(errno));
		return -errno;
	}

	s->stream.string_data = false;
	s->stream.notify_read = poe_stream_msg_cb;
	s->stream.notify_state = poe_stream_notify_cb;

	ustream_fd_init(s, tty);
	tcflush(tty, TCIFLUSH);

	return 0;
}

static int poet_setup(const struct port_config *ports, size_t num_ports)
{
	uint8_t port_ids[4], priorities[4], powerup_mode[4], limit_type[4];
	uint8_t enable_all[4] = {1, 1, 1, 1};
	size_t i = 0, num_okay = 0;

	do {
		for ( ; i < num_ports; i++) {
			if (!ports[i].enable)
				continue;

			port_ids[num_okay] = i;
			priorities[num_okay] = ports[i].priority;
			powerup_mode[num_okay] = ports[i].power_up_mode;
			limit_type[num_okay] = (ports[i].power_budget) ? 2 : 1;

			if (++num_okay == 4)
				break;
		};

		memset(enable_all + num_okay, 0xff, 4 - num_okay);
		memset(port_ids + num_okay, 0xff, 4 - num_okay);
		memset(priorities + num_okay, 0xff, 4 - num_okay);
		memset(powerup_mode + num_okay, 0xff, 4 - num_okay);
		memset(limit_type + num_okay, 0xff, 4 - num_okay);

		poe_set_port_priority(port_ids, priorities);
		//poe_set_port_power_up_mode(port_ids, powerup_mode);
		poe_cmd_port_classification(port_ids, enable_all);
		poe_cmd_port_power_limit_type(port_ids, limit_type);

		num_okay = 0;
	} while (++i < num_ports);

	return 0;
}

int poe_port_setup(void)
{
	size_t i;
	for (i = 0; i < config.port_count; i++) {
		if (!config.ports[i].enable || !config.ports[i].power_budget)
			continue;

		poe_cmd_port_disconnect_type(i, 2);
		poe_cmd_port_detection_type(i, 3);
		poe_cmd_port_power_budget(i, config.ports[i].power_budget);

	}

	poet_setup(config.ports, config.port_count);

	for (i = 0; i < config.port_count; i++)
		poe_cmd_port_enable(i, !!config.ports[i].enable);

	return 0;
}

static void poe_set_power_budget(const struct config *config)
{
	unsigned int pse;

	for (pse = 0; pse < 8; pse++) {
		if (!(config->pse_id_set_budget_mask & (1 << pse)))
			continue;

		poe_cmd_global_power_budget(pse, config->budget,
					    config->budget_guard);
	}
}

static int poe_initial_setup(void)
{
	poe_cmd_status();
	poe_cmd_power_mgmt_mode(2);
	poe_cmd_port_mapping_enable(false);
	poe_set_power_budget(&config);

	poe_port_setup();

	return 0;
}

static void state_timeout_cb(struct uloop_timeout *t)
{
	size_t i;
	
	poe_cmd_power_stats();
	for (i = 0; i < config.port_count; i++) {
		poe_cmd_port_config(i);
		poe_cmd_port_ext_config(i);
		poe_cmd_port_power_stats(i);
        poe_cmd_poe_status(i);
        poe_cmd_channel_status(i);
        poe_cmd_channel_pwr_status(i);

		//to syslog
		if(state.ports[i].status != state.ports[i].status_old){
			openlog("tfortis", LOG_PID, LOG_USER);
    		syslog(LOG_INFO, "realtek-poe: port %d changed: %s",i+1,state.ports[i].status_str);
			closelog();
			state.ports[i].status_old = state.ports[i].status;
 		}
	}
	uloop_timeout_set(t, 2 * 1000);
}



int main(int argc, char ** argv)
{
	int ch;

	struct uloop_timeout state_timeout = {
		.cb = state_timeout_cb,
	};

	struct ubus_auto_conn conn = {
		.cb = ubus_connect_handler,
	};

	ulog_open(ULOG_STDIO | ULOG_SYSLOG, LOG_DAEMON, "realtek-poe");
	ulog_threshold(LOG_INFO);
	
	

	while ((ch = getopt(argc, argv, "d")) != -1) {
		switch (ch) {
		case 'd':
			ulog_threshold(LOG_DEBUG);
			break;
		}
	}

	config_load(1);
	config_apply_quirks(&config);

	uloop_init();
	ubus_auto_connect(&conn);

	if (poe_stream_open("/dev/ttyS1", &stream, B115200) < 0){
		ULOG_DBG("ttyS1 not open\n");
		return -1;
	}

	poe_initial_setup();
	uloop_timeout_set(&state_timeout, 1000);
	uloop_run();
	uloop_done();

	return 0;
}