#!/usr/bin/lua

package.path = "/etc/tf_clish/scripts/vlan/?.lua;" .. package.path

local tf            = require "tf_module"
local utils         = require "tf_utilities"
local vlan_module   = require "vlan_module"
local br            = require "bridge_module"
local pm            = require "print_module"

local lanInfoMain = {}

local cmdList = {
    cmd_systemBoard       = "ubus -S call system board",
    cmd_systemBootloader  = "ubus -S call uci get \'{\"config\":\"system\",\"section\":\"tfortis\", \"option\":\"bootloader\"}\'",
    cmd_systemDescription = "ubus -S call uci get '{\"config\":\"system\",\"section\":\"tfortis\"}'",
    cmd_systemTimeInfo    = "ubus -S call system info",
    cmd_netIfInfo         = "ubus -S call network.interface dump",
    cmd_netInfo           = "ubus -S call uci get \'{\"config\":\"network\"}'",
    cmd_netLanInfo        = "ubus -S call uci get \'{\"config\":\"network\", \"section\":\"",
    cmd_netSwitchInfo     = "ubus -S call uci get \'{\"config\":\"network\",\"section\":\"switch\",\"option\":\"macaddr\"}\'",
}

local function printBootloaderVersion()
    local bootVer = tf.collectJsonTable(cmdList.cmd_systemBootloader)
    for key, value in pairs(bootVer) do
        if (key == "value") then
            print("\tBootloader Version:" .. "\t" .. value)
        end
    end
end

local function printSyStemBoard()
    print("System Info:")
    local systemBoard = tf.collectJsonTable(cmdList.cmd_systemBoard)
    if systemBoard == nil then
        print(pm.critical.board)
        return
    end

    if systemBoard["model"] ~= nil and systemBoard["model"] ~= "" then
        print(br.board_param.model .. systemBoard["model"])
    end

    if systemBoard["hostname"] ~= nil and systemBoard["hostname"] ~= ""  then
        print(br.board_param.hostname .. systemBoard["hostname"])
    end

    if systemBoard["release"] ~= nil then
        local release = systemBoard["release"]
        print(br.board_param.version .. release["version"])
    end
    --printBootloaderVersion()
end

local function printSyStemDescription()
    local systemInfo = tf.collectJsonTable(cmdList.cmd_systemDescription)
    if systemInfo == nil or systemInfo.values == nil then
        print(pm.critical.system)
        return
    end
    systemInfo = systemInfo.values

    if systemInfo["description"] ~= nil then
        print(br.system_param.description .. systemInfo["description"])
    else
        print(br.system_param.description .. "----- not set -------")
    end
    if systemInfo["contacts"] ~= nil then
        print(br.system_param.contacts .. systemInfo["contacts"])
    else
        print(br.system_param.contacts .. "----- not set -------")
    end
    if systemInfo["location"] ~= nil then
        print(br.system_param.location .. systemInfo["location"])
    else
        print(br.system_param.location .. "----- not set -------")
    end
end

local function printSystemInfo()
    print("Time Info:")
    local time_and_date = tf.executeCommand("date +\"%d %B %Y %T\"")
    local timeInfo = tf.collectJsonTable(cmdList.cmd_systemTimeInfo)
    if timeInfo == nil then
        print(pm.critical.time)
        return
    end
    local timeZoneInfo = tf.collectJsonTable(cmdList.cmd_systemDescription)
    if timeZoneInfo == nil or timeZoneInfo.values == nil then
        print(pm.critical.system)
        return
    end
    timeZoneInfo = timeZoneInfo.values

    if timeZoneInfo["zonename"] ~= nil then
        timeZoneInfo = timeZoneInfo["zonename"]
        local timeZone = tonumber(string.match(timeZoneInfo, "-?%d+"))
        if timeZone >= -12 and timeZone <= 12 then
            print(br.time_param.zonename .. timeZone)
        end
    end

    if time_and_date ~= nil then
        string.gsub(time_and_date, "\n", "")
        io.write(br.time_param.time_and_date, time_and_date)
    end
    if timeInfo["uptime"] ~= nil then
        local uptime = timeInfo["uptime"]
        local date = utils.convertSeconds(tonumber(uptime))
        print(br.time_param.uptime .. date.days .. " days and " .. date.hours .. ":" .. date.minutes .. ":" .. date.sec .. " (HH:MM:SS)")
    end
end

local function getNetMacInfo()
    local
    macInfo = tf.collectJsonTable(cmdList.cmd_netSwitchInfo)
    local data = {}
    data.str   = br.if_enum.mac_addr
    data.value = "----- not set -------"
    lanInfoMain["mac"] = data
    if macInfo == nil or macInfo.value == nil then
        return
    end
    data.value = macInfo.value
    lanInfoMain["mac"] = data
end

local function printNetInfo()
    local interfaceList = tf.collectJsonTable("ubus call network.interface dump")
    if interfaceList == nil or interfaceList["interface"] == nil then
        print(pm.critical.if_list)
        return 1
    else
        interfaceList = interfaceList["interface"]
    end

    for _, if_table in pairs(interfaceList) do
        if if_table["interface"] ~= "loopback" then
            print(" -------------------------------------------------- ")
            print("Interface:\t" .. if_table["interface"])
            if if_table["device"] ~= nil then
                print(br.if_enum.device .. if_table["device"])
            end
            if if_table["proto"] ~= nil then
                print(br.if_enum.proto .. if_table["proto"])
            end
            if if_table["ipv4-address"] ~= nil then
                local ipv4_m = if_table["ipv4-address"]
                print(br.if_enum.ipaddr)
                for _, ipv4_address in pairs(ipv4_m) do
                    if ipv4_address["address"] ~= nil then
                        print("\t\t\t" .. ipv4_address["address"] .. "/" .. ipv4_address["mask"])
                    end
                end
            end
        end
    end
    if lanInfoMain["mac"] ~= nil then
        print(tostring(lanInfoMain["mac"].str), lanInfoMain["mac"].value)
    end
end

local function printVlanId()
    local netInfo = tf.collectJsonTable(cmdList.cmd_netInfo)
    local vlanArr = vlan_module.getBridgeVlanList(netInfo["values"])
    local vlanId_str = ""
    if (vlanArr ~= nil) then
        for vlan_id, _ in pairs(vlanArr) do
            if string.len(vlanId_str) == 0 then
                vlanId_str = tostring(vlan_id)
            else
                vlanId_str = vlanId_str .. ", " .. tostring(vlan_id)
            end
        end
        print(br.if_enum.vlan_id .. vlanId_str)
    else
        print(br.if_enum.vlan_id .. "Vlan not configured correctly")
    end
end

local function run_show_switch()
    print(" -------------------------------------------------------------- ")
    printSyStemBoard()
    printSyStemDescription()
    printSystemInfo()
    getNetMacInfo()
    printNetInfo()
    printVlanId()
    print(" -------------------------------------------------------------- ")
end

run_show_switch()
