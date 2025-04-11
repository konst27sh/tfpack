//
// Created by sheverdin on 3/19/24.
//

#include "i2c_module.h"
#include "time.h"
#include "syslog.h"
#include "utils_lib/utils_module.h"
#include "events_lib/events_handlers.h"

int i2c_fd;

struct timer hi_prio_timer,low_prio_timer;

static uint8_t i2c_raw_data[1000];
static uint16_t hi_prio_len;
static uint16_t low_prio_len;



static event_t check_out_of_range(uint32_t value, uint32_t prevValue, uint32_t max, uint32_t min);
//static void i2c_sendError(I2C_ERROR_t *i2cData, REGISTER_ADDR addr, uint32_t value, I2C_EVENT_t i2c_event);

static event_t check_changed_state(uint32_t value, uint32_t preValue, uint32_t state_1, uint32_t state_2);
static uint16_t get_16bitValue(const uint8_t *val1);
static uint32_t get_32bitValue(const uint8_t *val1);

static uint8_t i2c_parse_tlv(uint16_t len);



const char i2c_eventDict[I2C_MAX_EVENT][I2C_ERROR_NAME_MAX_LEN] = {
    "\n",
    "ERR_OVER_MAX\n",
    "ERR_LESS_MIN\n",
    "LONG_PRESSED\n",
    "CHANGED_TO_VAC\n",
    "CHANGED_TO_BAT\n",
    "CHANGED_TO_OPEN\n",
    "CHANGED_TO_CLOSE\n",
    "empty\n",
    "empty\n",
    "CRITICAL_ERROR - RESET\n"
};

i2c_write_t i2c_write;

i2c_data_t sock_msgArr[MAX_SENSORS] =
{
    // MAIN
    {.addr = REG_INTSTAT, .name="INTSTAT", .category = none,   .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL},
    {.addr = REG_INTMASK, .name="INTMASK", .category = none,   .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL},
    {.addr = REG_HW_VERS, .name="HW_VERS", .category = none,   .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL},
    {.addr = REG_SW_VERS, .name="SW_VERS", .category = none,   .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL},
    {.addr = REG_ADC_CH1, .name="ADC_CH1", .category = none,   .lenData = 2, .type = TYPE_FLOAT_SFP, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = handler_ADC_CH1 },
    {.addr = REG_ADC_CH2, .name="ADC_CH2", .category = none,   .lenData = 2, .type = TYPE_FLOAT_SFP, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = handler_ADC_CH2 },
    {.addr = REG_ADC_CH3, .name="ADC_CH3", .category = none,   .lenData = 2, .type = TYPE_FLOAT_SFP, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = handler_ADC_CH3 },

    // SENSOR
    {.addr = REG_TAMPER,               .name="tamper",  .category = sensors,      .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = handler_TAMPER},
    {.addr = REG_SENSOR1,              .name="sensor1", .category = sensors,      .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = handler_SENSOR1},
    {.addr = REG_SENSOR2,              .name="sensor2", .category = sensors,      .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = handler_SENSOR2},
    {.addr = REG_RELAY1,              .name="relay",   .category = sensors,      .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = handler_RELAY1},
    {.addr = REG_DEFAULT_BUTTON,      .name="DEFAULT_BUTTON",      .category = none,      .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL},
    {.addr = REG_DEFAULT_LED,         .name="DEFAULT_LED",         .category = none,      .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL},
    {.addr = REG_DEFAULT_LONG_PRESSED,.name="DEFAULT_LONG_PRESSED",.category = none,.lenData = 2, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = handler_DEFAULT_LONG_PRESSED},
    {.addr = REG_READY_LED,           .name="READY_LED",           .category = none,      .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL},
    {.addr = REG_ALARM_LED,           .name="ALARM_LED",           .category = none,      .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL},

    // UPS
    {.addr = REG_RPS_CONNECTED,     .name="upsModeAvalible",  .category = ups, .lenData = 1, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL     },
    {.addr = REG_RPS_HW_VERS,       .name="upsHwVers",        .category = ups, .lenData = 1, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL       },
    {.addr = REG_RPS_SW_VERS,       .name="upsSwVers",        .category = ups, .lenData = 1, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL       },
    {.addr = REG_RPS_VAC,           .name="upsPwrSource",     .category = ups, .lenData = 1, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = handler_RPS_VAC           },
    {.addr = REG_RPS_BAT_VOLTAGE,   .name="upsBatteryVoltage",.category = ups, .lenData = 2, .type = TYPE_PSEUDO_FLOAT, .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = handler_RPS_BAT_VOLTAGE   },
    {.addr = REG_RPS_CHRG_VOLTAGE,  .name="upsChargeVoltage", .category = ups, .lenData = 2, .type = TYPE_PSEUDO_FLOAT, .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = handler_RPS_CHRG_VOLTAGE  },
    {.addr = REG_RPS_BAT_CURRENT,   .name="upsBatCurrent",    .category = ups, .lenData = 2, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = handler_RPS_BAT_CURRENT   },
    {.addr = REG_RPS_TEMPER,        .name="upsRpsTemper",     .category = ups, .lenData = 1, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,     .eventsHandler = handler_RPS_TEMPER        },
    {.addr = REG_RPS_LED_STATE,     .name="upsLedState",      .category = ups, .lenData = 1, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL     },
    {.addr = REG_RPS_BAT_KEY,       .name="upsBatKey",        .category = ups, .lenData = 1, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL },
    {.addr = REG_RPS_CHRG_KEY,      .name="upsChargeKey",     .category = ups, .lenData = 1, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL },
    {.addr = REG_RPS_REL_STATE,     .name="upsRelState",      .category = ups, .lenData = 1, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL },
    {.addr = REG_RPS_MIN_VOLTAGE,   .name="upsMinVoltage",    .category = ups, .lenData = 2, .type = TYPE_PSEUDO_FLOAT, .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL },
    {.addr = REG_RPS_DISCH_VOLTAGE, .name="upsDischargeVoltage",.category = ups, .lenData = 2, .type = TYPE_PSEUDO_FLOAT, .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL },
    {.addr = REG_RPS_REMAIN_TIME,   .name="upsBatteryTime",   .category = ups, .lenData = 2, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL },
    {.addr = REG_RPS_TEST_OK,       .name="upsRpsTestOk",     .category = ups, .lenData = 1, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL },
    {.addr = REG_RPS_CPU_ID,        .name="upsRpsTestOk",     .category = ups, .lenData = 2, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL },
    {.addr = REG_RPS_LTC4151_OK,    .name="upsLTC4151Ok",     .category = ups, .lenData = 1, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL },
    {.addr = REG_RPS_ADC_BAT_VOLT,  .name="upsAdcBatVoltage", .category = ups, .lenData = 2, .type = TYPE_PSEUDO_FLOAT, .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL },
    {.addr = REG_RPS_ADC_BAT_CURR,  .name="upsAdcBatCurr",    .category = ups, .lenData = 2, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL },
    {.addr = REG_RPS_TEST_MODE,     .name="upsRpsTestMode",   .category = ups, .lenData = 1, .type = TYPE_UINT,         .value = 0, .opcode = I2C_OPCODE_IDLE,       .eventsHandler = NULL },

    // SHT
    {.addr = REG_SHT_CONNECTED,     .name="sensorConnected",  .category = sensors,.lenData = 1, .type = TYPE_UINT,      .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = handler_SHT_CONNECTED   },
    {.addr = REG_SHT_TYPE,          .name="SHT_TYPE",         .category = sensors,.lenData = 10, .type = TYPE_UINT,     .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = handler_SHT_TYPE        },
    {.addr = REG_SHT_TEMPERATURE,   .name="sensorTemperature",.category = sensors,.lenData = 2, .type = TYPE_FLOAT_SFP, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = handler_SHT_TEMPERATURE },
    {.addr = REG_SHT_HUMIDITY,      .name="sensorHumidity",   .category = sensors,.lenData = 1, .type = TYPE_UINT,      .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = handler_SHT_HUMIDITY    },


    // SFP_1
    {.addr = REG_SFP1_PRESENT,      .name="portSfpPresent_1",     .category = sfp1,.lenData = 1,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = handler_SFP1_PRESENT },
    {.addr = REG_SFP1_LOS,          .name="portSfpSignalDetect_1",.category = sfp1,.lenData = 1,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = handler_SFP1_LOS     },
    {.addr = REG_SFP1_VENDOR,       .name="portSfpVendor_1",      .category = sfp1,.lenData = 16, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_VENDOR_OUI,   .name="portSfpOui_1",         .category = sfp1,.lenData = 3,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_VENDOR_PN,    .name="portSfpPartNumber_1",  .category = sfp1,.lenData = 16, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_VENDOR_REV,   .name="portSfpRevision_1",    .category = sfp1,.lenData = 4,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_IDENTIFIER,   .name="portSfpIdentifier_1",  .category = sfp1,.lenData = 4,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_CONNECTOR,    .name="portSfpConnector_1",   .category = sfp1,.lenData = 30, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_TYPE,         .name="portSfpType_1",        .category = sfp1,.lenData = 16, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_LINK_LEN,     .name="portSfpLinkLen_1",     .category = sfp1,.lenData = 25, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_FIBER_TEC,    .name="portSfpFiberTec_1",    .category = sfp1,.lenData = 32, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_MEDIA,        .name="portSfpMedia_1",       .category = sfp1,.lenData = 32, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_SPEED,        .name="portSfpSpeed_1",       .category = sfp1,.lenData = 16, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_ENCODING,     .name="portSfpEncoding_1",    .category = sfp1,.lenData = 16, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_LENC,         .name="portSfpLenC_1",        .category = sfp1,.lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_WAVELEN,      .name="portSfpWavelen_1",     .category = sfp1,.lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_NBR,          .name="portSfpNBR_1",         .category = sfp1,.lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_LEN9,         .name="portSfpLen9_1",        .category = sfp1,.lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_LEN50,        .name="portSfpLen50_1",       .category = sfp1,.lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_LEN62,        .name="portSfpLen62_1",       .category = sfp1,.lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_TEMPER,       .name="portSfpTemperature_1", .category = sfp1,.lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = handler_SFP1_TEMPER   },
    {.addr = REG_SFP1_VOLTAGE,      .name="portSfpVoltage_1",     .category = sfp1,.lenData = 2,  .type = TYPE_PSEUDO_FLOAT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = handler_SFP1_VOLTAGE  },
    {.addr = REG_SFP1_CURRENT,      .name="portSfpCurrent_1",     .category = sfp1,.lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_TX_BIAS,      .name="portSfpBiasCurrent_1", .category = sfp1,.lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL  },
    {.addr = REG_SFP1_TX_POWER_DB,  .name="portSfpTxOutPowerDb_1",.category = sfp1,.lenData = 2, .type = TYPE_FLOAT_SFP, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = handler_SFP1_TX_POWER_DB },
    {.addr = REG_SFP1_RX_POWER_DB,  .name="portSfpRxOutPowerDb_1",.category = sfp1,.lenData = 2, .type = TYPE_FLOAT_SFP, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = handler_SFP1_RX_POWER_DB },
    {.addr = REG_SFP1_TX_POWER,     .name="portSfpTxOutPower_1",  .category = sfp1,.lenData = 2, .type = TYPE_UINT,   .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_SFP1_RX_POWER,     .name="portSfpRxOutPower_1",  .category = sfp1,.lenData = 2, .type = TYPE_UINT,   .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    //SPF_2
    {.addr = REG_SFP2_PRESENT,      .name="portSfpPresent_2",     .category = sfp2,  .lenData = 1,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler =  handler_SFP2_PRESENT },
    {.addr = REG_SFP2_LOS,          .name="portSfpSignalDetect_2",.category = sfp2,  .lenData = 1,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler =  handler_SFP2_LOS     },
    {.addr = REG_SFP2_VENDOR,       .name="portSfpVendor_2",      .category = sfp2,  .lenData = 16, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL  },
    {.addr = REG_SFP2_VENDOR_OUI,   .name="portSfpOui_2",         .category = sfp2,  .lenData = 3,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL  },
    {.addr = REG_SFP2_VENDOR_PN,    .name="portSfpPartNumber_2",  .category = sfp2,  .lenData = 16, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL  },
    {.addr = REG_SFP2_VENDOR_REV,   .name="portSfpRevision_2",    .category = sfp2,  .lenData = 4,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL  },
    {.addr = REG_SFP2_IDENTIFIER,   .name="portSfpIdentifier_2",  .category = sfp2,  .lenData = 4,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL  },
    {.addr = REG_SFP2_CONNECTOR,    .name="portSfpConnector_2",   .category = sfp2,  .lenData = 30, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL  },
    {.addr = REG_SFP2_TYPE,         .name="portSfpType_2",        .category = sfp2,  .lenData = 16, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL  },
    {.addr = REG_SFP2_LINK_LEN,     .name="portSfpLinkLen_2",     .category = sfp2,  .lenData = 25, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL  },
    {.addr = REG_SFP2_FIBER_TEC,    .name="portSfpFiberTec_2",    .category = sfp2,  .lenData = 32, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },
    {.addr = REG_SFP2_MEDIA,        .name="portSfpMedia_2",       .category = sfp2,  .lenData = 32, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },
    {.addr = REG_SFP2_SPEED,        .name="portSfpSpeed_2",       .category = sfp2,  .lenData = 16, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },
    {.addr = REG_SFP2_ENCODING,     .name="portSfpEncoding_2",    .category = sfp2,  .lenData = 16, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },
    {.addr = REG_SFP2_WAVELEN,      .name="portSfpWavelen_2",     .category = sfp2,  .lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },
    {.addr = REG_SFP2_NBR,          .name="portSfpNBR_2",         .category = sfp2,  .lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },
    {.addr = REG_SFP2_LEN9,         .name="portSfpLen9_2",        .category = sfp2,  .lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },
    {.addr = REG_SFP2_LEN50,        .name="portSfpLen50_2",       .category = sfp2,  .lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },
    {.addr = REG_SFP2_LEN62,        .name="portSfpLen62_2",       .category = sfp2,  .lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },
    {.addr = REG_SFP2_LENC,         .name="portSfpLenC_2",        .category = sfp2,  .lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },
    {.addr = REG_SFP2_TEMPER,       .name="portSfpTemperature_2", .category = sfp2,  .lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = handler_SFP2_TEMPER   },
    {.addr = REG_SFP2_VOLTAGE,      .name="portSfpVoltage_2",     .category = sfp2,  .lenData = 2,  .type = TYPE_PSEUDO_FLOAT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = handler_SFP2_VOLTAGE  },
    {.addr = REG_SFP2_CURRENT,      .name="portSfpCurrent_2",     .category = sfp2,  .lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },
    {.addr = REG_SFP2_TX_BIAS,      .name="portSfpBiasCurrent_2", .category = sfp2,  .lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },
    {.addr = REG_SFP2_TX_POWER_DB,  .name="portSfpTxOutPowerDb_2",.category = sfp2,  .lenData = 2,  .type = TYPE_FLOAT_SFP, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = handler_SFP2_TX_POWER_DB },
    {.addr = REG_SFP2_RX_POWER_DB,  .name="portSfpRxOutPowerDb_2",.category = sfp2,  .lenData = 2,  .type = TYPE_FLOAT_SFP, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = handler_SFP2_RX_POWER_DB },
    {.addr = REG_SFP2_TX_POWER,     .name="portSfpTxOutPower_2",  .category = sfp2,  .lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },
    {.addr = REG_SFP2_RX_POWER,     .name="portSfpRxOutPower_2",  .category = sfp2,  .lenData = 2,  .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,   .eventsHandler = NULL },

    // RTC
    {.addr = REG_RTC_STATUS, .name="RTC_STATUS", .category = rtc, .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_RTC_YEAR,   .name="RTC_YEAR",   .category = rtc, .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_RTC_MONTH,  .name="RTC_MONTH",  .category = rtc, .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_RTC_DAY,    .name="RTC_DAY",    .category = rtc, .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_RTC_WEEKDAY,.name="RTC_WEEKDAY",.category = rtc, .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_RTC_HOUR,   .name="RTC_HOUR",  .category = rtc, .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_RTC_MINUTE, .name="RTC_MINUTE",.category = rtc, .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_RTC_SECOND, .name="RTC_SECOND",.category = rtc, .lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },

    // POE
    {.addr = REG_POE_ID,   .name="POE_ID",   .category = none,.lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_POE_STATE,.name="POE_STATE",.category = none,.lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_POE_BANK, .name="POE_BANK", .category = none,.lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
    {.addr = REG_POE_MODE, .name="POE_MODE", .category = none,.lenData = 1, .type = TYPE_UINT, .value = 0, .opcode = I2C_OPCODE_IDLE,  .eventsHandler = NULL },
};

void i2c_test(void)
{
    printf("Hello from I2C\n");
}

REGISTER_ADDR get_i2c_addr_by_name(const char *reg_name)
{
    for (uint8_t i = 0; i < sizeof(sock_msgArr)/sizeof(i2c_data_t); i++) {
        //printf("i = %d reg_name = %s -- i2c_name %s\n", i, reg_name, sock_msgArr[i].name);
        if (strcmp(reg_name, sock_msgArr[i].name) == 0) {
            //printf("FOUND i = %d gpio_name = %s\n", i, sock_msgArr[i].name);
            return sock_msgArr[i].addr;
        }
    }
    return 0;
}

void open_i2c()
{
    i2c_fd = open(I2C_ADAPTER, O_RDWR);
    if (i2c_fd < 0) {
        perror("open i2c");
        LOG_DBG(ERROR,"Unable to open i2c file\n");
        exit (EXIT_FAILURE);
    }
    LOG_DBG(DEBUG_NORM,"OPEN I2C PORT\n");

    timer_set(&hi_prio_timer,3);
    timer_set(&low_prio_timer,0);

    read_buffer(i2c_fd,REG_HI_PRIO_LEN,2,i2c_raw_data);
    hi_prio_len = i2c_raw_data[1]<<8 | i2c_raw_data[0];


    read_buffer(i2c_fd,REG_LOW_PRIO_LEN,2,i2c_raw_data);
    low_prio_len = i2c_raw_data[1]<<8 | i2c_raw_data[0];


    if(isDebugMode()) {
        LOG_DBG(DEBUG_NORM,"hi_prio_len = %d\n",hi_prio_len);
        LOG_DBG(DEBUG_NORM,"low_prio_len = %d\n",low_prio_len);
    }
}


int is_port_opened(){
    if(i2c_fd)
        return 1;
    else
        return 0;
}

void close_i2c() {
    LOG_DBG(DEBUG_NORM,"I2C PORT CLOSED \n");
    close(i2c_fd);
    i2c_fd = 0;
}

uint8_t read_i2c(){
    uint8_t tmp,errorCode = 0;

    if(timer_expired(&hi_prio_timer)){
        if(is_port_opened()) {
            errorCode = read_buffer(i2c_fd, REG_HI_PRIO_DUMP, hi_prio_len, i2c_raw_data);
            if (errorCode == 0) {
                errorCode = i2c_parse_tlv(hi_prio_len);
                if (isDebugMode()) {
                    LOG_DBG(DEBUG_VERBOSE, "Read Hi prio %d\n", hi_prio_len);
                }
            }
        }
        timer_set(&hi_prio_timer,3);
    }
    else if(timer_expired(&low_prio_timer)){
        if(is_port_opened()) {
            errorCode = read_buffer(i2c_fd, REG_LOW_PRIO_DUMP, low_prio_len, i2c_raw_data);
            if (errorCode == 0) {
                errorCode = i2c_parse_tlv(low_prio_len);
                if (isDebugMode()) {
                    LOG_DBG(DEBUG_VERBOSE, "Read Low prio %d\n", low_prio_len);
                }
            }
        }
        timer_set(&low_prio_timer,30);
    }
    else if(i2c_write.write_flag){

        LOG_DBG(DEBUG_NORM,"write_flag\n");
        LOG_DBG(DEBUG_NORM,"addr = %d\n", i2c_write.i2c_data.addr);
        LOG_DBG(DEBUG_NORM,"len = %d\n", i2c_write.i2c_data.lenData);
        LOG_DBG(DEBUG_NORM,"value[0] = %d\n", i2c_write.i2c_data.value[0]);
        if(is_port_opened()) {
            write_buffer(i2c_fd, i2c_write.i2c_data.addr, i2c_write.i2c_data.lenData, i2c_write.i2c_data.value);
        }
        i2c_write.write_flag = 0;
    }
    return errorCode;
}

static uint8_t  get_id_by_addr(REGISTER_ADDR addr){
    for (uint8_t i = 0; i < sizeof(sock_msgArr)/sizeof(i2c_data_t); i++) {
        if(sock_msgArr[i].addr == addr)
            return i;
    }
}


static uint8_t i2c_parse_tlv(uint16_t len){
    uint16_t offset = 0;
    uint8_t id;
    i2c_tlv *tlv;
    while(offset < len){
        tlv = (i2c_tlv *)((uint8_t *) i2c_raw_data + offset);
        if(tlv->regaddr > MAX_SENSORS){
            LOG_DBG(DEBUG_NORM,"Read: reg addr overflow!\n");
            return EXIT_FAILURE;
        }
        id = get_id_by_addr(tlv->regaddr);
        if(tlv->len < sizeof(sock_msgArr[id].value)){
            memcpy(sock_msgArr[id].value,tlv->data,tlv->len);
        }
        offset += tlv->len+2;
    }
    return EXIT_SUCCESS;
}



//read I2C data
int read_buffer(int fd, unsigned char regaddr, int len, unsigned char *buffer)
{
        struct i2c_rdwr_ioctl_data data;
        struct i2c_msg messages[2];
        unsigned char regaddr_[2];


        regaddr_[0] = regaddr;
        regaddr_[1] = len;

        /*
         * .addr -
         * .flags -  (0 - w, 1 - r)
         * .len -
         * .buf -
         */
        messages[0].addr = I2C_ADDR;
        messages[0].flags = 0;//write
        messages[0].len = 2;
        messages[0].buf = regaddr_;

        messages[1].addr = I2C_ADDR;
        messages[1].flags = 1;//read
        messages[1].len = len;
        messages[1].buf = buffer;

        data.msgs = messages;
        data.nmsgs = 2;
   
        if (ioctl(fd, I2C_RDWR, &data) < 0){
            LOG_DBG(DEBUG_NORM,"Read: cant send data!\n");
            return EXIT_FAILURE;
        }        
        if (buffer[0] == 0xAA && buffer[1] == 0xAA)
        {
            LOG_DBG(ERROR,"I2C: error response\n");
            return EXIT_FAILURE;
        }
  
        return EXIT_SUCCESS;
}


//write I2C data
int write_buffer(int fd, unsigned char regaddr, int len, unsigned char *buffer)
{
    unsigned char tmp[10];
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg messages[1];

    if(isDebugMode()) {
        LOG_DBG(DEBUG_NORM,"write_buffer\n");
        LOG_DBG(DEBUG_NORM,"addr = %d\n", regaddr);
        LOG_DBG(DEBUG_NORM,"len = %d\n", len);
        LOG_DBG(DEBUG_NORM,"value[0] = %d\n", buffer[0]);
    }

    //printf("value[1] = %d\n",buffer[1]);
    //printf("value[2] = %d\n",buffer[2]);
    //printf("value[3] = %d\n",buffer[3]);

    /*
     * .addr -
     * .flags -  (0 - w, 1 - r)
     * .len -
     * .buf -
     */

    messages[0].addr = I2C_ADDR;
    messages[0].flags = 0;//write
    messages[0].len = len+2;
    messages[0].buf = tmp;
    messages[0].buf[0] = regaddr;
    messages[0].buf[1] = len;
    for(int i=0; i < len; i++)
        messages[0].buf[i+2] = buffer[i];
    
    data.msgs = messages;
    data.nmsgs = 1;
    
    if (ioctl(fd, I2C_RDWR, &data) < 0){
        LOG_DBG(ERROR,"Write: cant send data!\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int i2c_get_MsgSize(void)
{
    int len = sizeof(i2c_data_t);
    return len;
}

void i2c_getData(i2c_data_t *i2CData, REGISTER_ADDR addr, int len)
{
    uint8_t id;
    id = get_id_by_addr(addr);
    sock_msgArr[id].lenMSG = len;
    memcpy(i2CData, &sock_msgArr[id], len);
}

//todo
void i2c_setData(i2c_data_t *i2CData)
{
    if(isDebugMode()) {
        LOG_DBG(DEBUG_NORM,"i2c_setData\n");
        LOG_DBG(DEBUG_NORM,"addr = %d\n",i2CData->addr);
        LOG_DBG(DEBUG_NORM,"len = %d\n",i2CData->lenData);
        LOG_DBG(DEBUG_NORM,"value[0] = %d\n",i2CData->value[0]);
        LOG_DBG(DEBUG_NORM,"value[1] = %d\n",i2CData->value[1]);
        LOG_DBG(DEBUG_NORM,"value[2] = %d\n",i2CData->value[2]);
        LOG_DBG(DEBUG_NORM,"value[3] = %d\n",i2CData->value[3]);
    }
    memcpy(&i2c_write.i2c_data, i2CData, sizeof(i2c_write.i2c_data));
    i2c_write.write_flag = 1;
}

void i2c_events_handler() {
    static uint8_t i;
    static uint8_t delayStart = 0;
    i2c_event_data_t i2cData;
    uint8_t pause_sec = 3;
    for (i = 0; i < sizeof(sock_msgArr)/sizeof(i2c_data_t); i++) {
        i2cData.addr = sock_msgArr[i].addr;
        memcpy(i2cData.value, sock_msgArr[i].value, sizeof(i2cData.value));
        if(sock_msgArr[i].eventsHandler != NULL)
            sock_msgArr[i].eventsHandler(&i2cData);
    }
}






void i2c_parsingData(REGISTER_ADDR addr,char *str)
{
    uint8_t id;
    uint16_t tmp16;
    int16_t itmp16;

    id = get_id_by_addr(addr);

    switch (sock_msgArr[id].lenData)
    {
        case 1:
            //uint8
            sprintf(str,"%d", sock_msgArr[id].value[0]);
            break;
        case 2:
        {
            if (sock_msgArr[id].type == TYPE_PSEUDO_FLOAT)
            {
                tmp16 = sock_msgArr[id].value[0] | sock_msgArr[id].value[1] << 8;
                if (tmp16 < 1000)
                    sprintf(str,"%d.%d", tmp16 / 10, tmp16 % 10);
                else
                    sprintf(str,"%d.%03d", tmp16 / 1000, tmp16 % 1000);
            }
            else  if (sock_msgArr[id].type == TYPE_FLOAT_SFP)
            {
                itmp16 = sock_msgArr[id].value[0] | sock_msgArr[id].value[1]<< 8;
                if (itmp16 < 1000)
                    sprintf(str,"%d.%d", itmp16 / 10, abs(itmp16 % 10));
                else
                    sprintf(str,"%d.%03d", itmp16 / 1000, abs(itmp16 % 1000));
            }
            else
            {
                //uint16
                sprintf(str,"%d", sock_msgArr[id].value[0] | sock_msgArr[id].value[1] << 8);
            }
        }
            break;
        case 4:
            //uint32
            sprintf(str,"%d", (uint32_t) (sock_msgArr[id].value[0] |
                                       sock_msgArr[id].value[1] << 8 |
                                       sock_msgArr[id].value[2] << 16 |
                                       sock_msgArr[id].value[3] << 24));
            break;
        default:
            sprintf(str,"%s", sock_msgArr[id].value);
    }
}


register_category_t get_param_category(const char *name){
    if(!strlen(name))
        return none;
    if(strcmp(name,"ups")==0)
        return ups;
    if(strcmp(name,"sensors")==0)
        return sensors;
    if(strcmp(name,"sfp1")==0)
        return sfp1;
    if(strcmp(name,"sfp2")==0)
        return sfp2;
    if(strcmp(name,"rtc")==0)
        return rtc;
}


