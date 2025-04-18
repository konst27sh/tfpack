#!/usr/bin/lua

package.path = "/etc/tf_clish/scripts/lldpd/?.lua;" .. package.path

local tf          = require "tf_module"
local utils       = require "lldp_utils"
local neighbors   = require "lldp_neighbors"
local statistics  = require "lldp_statistics"
local interface   = require "lldp_interfaces"
local chassis     = require "lldp_chassis"

local portRange = { 0, 0 }
local lldpdInfo = {}
local lldpConfig = {}

local interfaces_e = {}

local cmdList = {
    lldpd_status = "ubus call lldpd getStatus",
    lldpd_config = "ubus call uci get '{\"config\":\"lldpd\"}'"
}

local function Interfaces(isPrint)
    print(">>> Interfaces: ")
    local i_portCount = 0
    for _, lldp in pairs(lldpdInfo.interfaces.lldp) do
        for _, lldp_interface_arr in pairs(lldp) do
            for _, lldp_interface in pairs(lldp_interface_arr) do
                if (lldp_interface ~= nil) then
                    if portRange[3] == "range" or portRange[3] == "once" then
                        isPrint = tf.needToPrint(portRange, lldp_interface.name)
                    elseif portRange[3] == "all" then
                        isPrint = 1
                    end
                    if isPrint then
                        i_portCount = interface.parsingInterfaces(lldp_interface)
                    end
                end
            end
        end
    end
    if i_portCount == 0 then
        print("info: according to your conditions, no PORT have been found for Interface statistics")
    end
end

local function Statistics(isPrint)
    print(">>> Statistics: ")
    local s_portCount = 0
    for i, lldp in pairs(lldpdInfo.statistics.lldp) do
        for key, lldp_statistics_arr in pairs(lldp) do
            for param, lldp_statistics in pairs(lldp_statistics_arr) do
                if portRange[3] == "all" then
                    s_portCount = statistics.parsingStatistics(lldp_statistics)
                else
                    isPrint = tf.needToPrint(portRange, lldp_statistics.name)
                    if isPrint then
                        s_portCount = statistics.parsingStatistics(lldp_statistics)
                    end
                end
            end
        end
    end
    if s_portCount == 0 then
        print("info: according to your conditions, Statistics are not presented")
    end
end

local function Neighbors(isPrint)
    print(">>> Neighbors: ")
    local n_portCount = 0

    for _, lldp in pairs(lldpdInfo.neighbors.lldp) do
        for _, lldp_neighbors_arr in pairs(lldp) do
            for _, lldp_neighbors in pairs(lldp_neighbors_arr) do
                if (lldp_neighbors ~= nil) then
                    if portRange[3] == "all" then
                        n_portCount = chassis.parsingChassis(lldp_neighbors, true)
                    else
                        isPrint = tf.needToPrint(portRange, lldp_neighbors.name)
                        if isPrint then
                            n_portCount = chassis.parsingChassis(lldp_neighbors, true)
                        end
                    end
                end
            end
        end
    end
    if n_portCount == 0 then
        print("\tNo remote Neighbors have been found")
    end
end

local function Chassis()
    print(">>> Chassis")
    for _, local_chassis_str in pairs(lldpdInfo.chassis["local-chassis"]) do
        if local_chassis_str ~= nil then
            chassis.parsingChassis(local_chassis_str, false)
        else
            print("error: local chassis  is empty")
        end
    end
end

local logic_enum =
{
    ["0"] = "no",
    ["1"] = "yes",
}

local function lldp_show_configuration()
    lldpConfig = tf.collectJsonTable(cmdList.lldpd_config)
    if lldpConfig == nil then
        print("error: lldp config is wrong")
        return
    end
    lldpConfig = lldpConfig.values.config
    if lldpConfig == nil then
        print("error: lldp config is wrong")
        return
    end

    print("State:")
    for _, p in pairs(utils.configList) do
        if lldpConfig[p.name] ~= nil then
            if p.name == "enable_lldp" or p.name == "force_lldp" then
                local value = lldpConfig[p.name]
                value = logic_enum[value]
                print(p.str .. value)
            end
        end
    end

    print("Protocols:")
    for _, p in pairs(utils.configList) do
        if p.name == "enable_fdp" or
            p.name == "enable_cdp" or
            p.name == "enable_sonmp" or
            p.name == "enable_edp"
        then
            local value = lldpConfig[p.name]
            value = logic_enum[value]
            print(p.str .. value)

        end
    end

    print("Parameters:")
    for _, p in pairs(utils.configList) do
        if p.name == "lldp_tx_interval" or
            p.name == "lldp_tx_hold" or
            p.name == "readonly_mode"
        then
            if lldpConfig[p.name] ~= nil then
                local value = lldpConfig[p.name]
                if p.name == "readonly_mode" then
                    value = logic_enum[value]
                end
                print(p.str .. value)
            else
                print(p.str .. "not set")
            end
        end
    end

    print("Interface:")
    if lldpConfig["interface"] == nil then
        print("\t not set")
        return
    end
    local ports = lldpConfig["interface"]
    for i, lan in pairs(ports) do
        print("\t\t" .. lan)
    end
end

local function lldp_show_status()
    if #arg < 3 then
        print("critical: lldp command not valid")
        return 2
    end
    local cmd_type = arg[3]
    local portRange_in = arg[2]
    portRange = tf.checkPortRange(portRange_in)
    if portRange == nil then
        print("critical: lldp ports range not valid")
        return
    end

    lldpdInfo = tf.collectJsonTable(cmdList.lldpd_status)
    lldpConfig = tf.collectJsonTable(cmdList.lldpd_config)
    lldpConfig = lldpConfig.values.config

    local isPrint = false
    if lldpConfig.interface == nil then
        --print("error: lldp config interface is empty")
        isPrint = true
    end
    local lldp_state = ""
    if lldpConfig.enable_lldp ~= nil and lldpConfig.enable_lldp == "0" then
        --print("error: lldp config interface is empty")
        lldp_state = "disable"
        print("info: lldp state: " .. lldp_state)
    end
    if cmd_type == "local" then
        if lldpdInfo.chassis == nil then
            print("info: lldp chassis - No data to display")
            return
        end
        Chassis(isPrint)
    elseif cmd_type == "remote" then
        if lldpdInfo.neighbors == nil then
            print("info: lldp neighbors - No data to display")
            return
        end
        Neighbors(isPrint)
    elseif cmd_type == "statistics" then
        if lldpdInfo.statistics == nil then
            print("info: lldp statistics - No data to display")
            return
        end
        Statistics(isPrint)
    elseif cmd_type == "interfaces" then
        if lldpdInfo.interfaces == nil then
            print("info: lldp interfaces - No data to display")
            return
        end
        Interfaces(isPrint)
    else
        print("error: lldp command is wrong")
    end
end

local lldp_enum = {
    configuration   = { func = lldp_show_configuration },
    status          = { func = lldp_show_status }
}

local function run_show_lldp()

    local cmd_in = arg[1]
    if lldp_enum == nil or lldp_enum[cmd_in] == nil then
        print("critical: lldp command is wrong")
        return
    end
    lldp_enum[cmd_in].func()
end

run_show_lldp()



