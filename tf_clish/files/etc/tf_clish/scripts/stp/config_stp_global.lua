#!/usr/bin/lua


package.path = "/etc/tf_clish/scripts/stp/?.lua;" .. package.path

local uci = require("luci.model.uci").cursor()
local tf = require "tf_module"
local stp = require "stp_module"

local function configState()
    local stp_state_err = 1
    local value = tonumber(arg[2])
    if value == 0 or value == 1 then
        uci:set("network", "switch", "stp", tostring(arg[2]))
        stp_state_err = 0
    else
        print("error: State value not corrected")
    end
    return stp_state_err
end

local function configLogLevel()
    local stp_LogLevel_err = 1
    local value = tonumber(arg[2])
    if value >= 0 and value <= 4 then
        uci:set("mstpd", "global", "loglevel", tostring(arg[2]))
        stp_LogLevel_err = 0
    else
        print("error: Log level not corrected")
    end
    return stp_LogLevel_err
end

local function configPriority()
    local stp_Priority_err = 1
    local value = tonumber(arg[2])
    if value >= 0 and value <= 15 then
        uci:set("mstpd", "switch", "treeprio", tostring(arg[2]))
        stp_Priority_err = 0
    else
        print("error: Priority value not corrected")
    end
    return stp_Priority_err
end

local function configProtocol()
    local stp_Protocol_err = 1
    if arg[2] == "stp" or arg[2] == "rstp" then
        uci:set("mstpd", "switch", "forcevers", tostring(arg[2]))
        stp_Protocol_err = 0
    else
        print("error: Protocol value not corrected")
    end
    return stp_Protocol_err
end

local function configAgeing()
    local value = tonumber(arg[2])
    local stp_aging_err = 1
    if value >= 10 and value <= 1000000 then
        uci:set("mstpd", "switch", "ageing", tostring(arg[2]))
        stp_aging_err = 0
    else
        print("error: Ageing value must be in range 10 – 1.0000.00 seconds")
    end
    return stp_aging_err
end

local function configHello_time()
    local stp_helloTime_err = 1
    local value = tonumber(arg[2])
    if value >= 1 and value <= 10 then
        uci:set("mstpd", "switch", "hello", tostring(arg[2]))
        stp_helloTime_err = 0
    else
        print("error: Hello time must be in range 1 – 10 seconds")
    end
    return stp_helloTime_err
end

local function configForward_delay()
    local stp_Forward_delay_err = 1
    local value = tonumber(arg[2])
    if value >= 4 and value <= 30 then
        uci:set("mstpd", "switch", "fdelay", tostring(arg[2]))
        stp_Forward_delay_err = 0
    else
        print("error: Forward delay time must be in range 4 - 30 seconds")
    end
    return stp_Forward_delay_err
end

local function configMax_age()
    local stp_Max_age_err = 1
    local value = tonumber(arg[2])
    if value >= 6 and value <= 40 then
        uci:set("mstpd", "switch", "maxage", tostring(arg[2]))
        stp_Max_age_err = 0
    else
        print("error: Max age time must be in range 6 - 40 seconds")
    end
end

local function configTX_hold_count()
    local stp_TX_hold_count_err = 1
    local value = tonumber(arg[2])
    if value >= 1 and value <= 10 then
        uci:set("mstpd", "switch", "txholdcount", tostring(arg[2]))
        stp_TX_hold_count_err = 0
    else
        print("error: TX hold count must be in range 1 - 10 seconds")
    end
    return stp_TX_hold_count_err
end

local stp_cmdList =
{
    {cmd = stp.cmd_paramList.state,         func = configState },
    {cmd = stp.cmd_paramList.loglevel,      func = configLogLevel },
    {cmd = stp.cmd_paramList.priority,      func = configPriority },
    {cmd = stp.cmd_paramList.protocol,      func = configProtocol },
    {cmd = stp.cmd_paramList.ageing,        func = configAgeing },
    {cmd = stp.cmd_paramList.hellotime,     func = configHello_time },
    {cmd = stp.cmd_paramList.forwarddelay,  func = configForward_delay },
    {cmd = stp.cmd_paramList.maxage,        func = configMax_age },
    {cmd = stp.cmd_paramList.txholdcount,   func = configTX_hold_count }
}

local function run_config_stp()
    for _, value in pairs(stp_cmdList) do
        if arg[1] == value.cmd then
            local res = value.func()
            if res == 0 then
                print("info: set " .. arg[1] .. " successful !")
            end
        end
    end
end

run_config_stp()


