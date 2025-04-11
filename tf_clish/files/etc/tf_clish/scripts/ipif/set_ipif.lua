#!/usr/bin/lua

---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by sheverdin.
--- DateTime: 6/3/24 2:18 PM
---

local tf = require "tf_module"
local pm = require "print_module"

local cmd_network_ubus = "ubus -S call uci get '{\"config\":\"network\", \"type\":\"interface\"}'"
local cmd_ubus_set = "ubus -S call uci set '{\"config\":\"network\",\"type\":\"interface\",\"section\":\""
local cmd_ubus_del = "ubus -S call uci delete '{\"config\":\"network\",\"type\":\"interface\",\"section\":\""

local ipif_paramList = {
    ["ifname"]  = "",
    ["proto"]   = "",
    ["ip"]      = "",
    ["netmask"] = "",
    ["gateway"] = "",
    ["dns"]     = "",
}

local function getparam()
    local argList = {}
    local arg_i = ""
    for i = 1, #arg do
        arg_i = arg[i]
        if ipif_paramList[arg_i] ~= nil then
            ipif_paramList[arg_i] = arg[i + 1]
        end
    end
    return argList
end

local function check_ip(input_ip)
    if tf.isValidIP(input_ip) then
        return "ip"
    elseif string.sub(input_ip, 1, 1) == "/" and #input_ip > 1 then
        return "mask"
    else
        return "error"
    end
end

local function static_handler(if_section, s)

    local network_ubus_cmd = ""
    local ubus_set_2 = "\","

    local ubus_set_value = "\"values\":{"
    local ubus_set_value_1 = ""

    local count = 0
    local ubus_set_value_list = {}
    count = count + 1
    ubus_set_value_list[count] = "\"proto\":\"" .. ipif_paramList.proto .. "\""

    local ip        = ""
    local mask      = ""
    local gateway   = ""
    local dns       = ""

    if ipif_paramList.ip ~= "" and ipif_paramList.ip ~= nil then
        if check_ip(ipif_paramList.ip) == "ip" then
            count = count + 1
            ip = ipif_paramList.ip
            ubus_set_value_list[count] = "\"ipaddr\":\"" .. ip .. "\""
        else
            print("critical: IP not valid")
        end
    end

    if ipif_paramList.netmask ~= "" and ipif_paramList.netmask ~= nil then
        local mask_res = check_ip(ipif_paramList.netmask)
        if mask_res == "ip" then
            count = count + 1
            mask = ipif_paramList.netmask
        elseif mask_res == "mask" then
            count = count + 1
            mask = string.sub(ipif_paramList.netmask, 2)
        else
            print("critical: Netmask not valid")
        end
        ubus_set_value_list[count] = "\"netmask\":\"" .. mask .. "\""
    end

    if ipif_paramList.gateway ~= "" and ipif_paramList.gateway ~= nil then
        if check_ip(ipif_paramList.gateway) == "ip" then
            count = count + 1
            gateway = ipif_paramList.gateway
            ubus_set_value_list[count] = "\"gateway\":\"" .. gateway .. "\""
        else
            print("critical: GATEWAY not valid")
        end
    end

    if ipif_paramList.dns ~= "" and ipif_paramList.dns ~= nil then
        if check_ip(ipif_paramList.dns) == "ip" then
            count = count + 1
            dns = ipif_paramList.dns
            ubus_set_value_list[count] = "\"dns\":\"" .. dns .. "\""
        else
            print("critical: DNS not valid")
        end
    end

    if ip == "" and if_section["ipaddr"] == nil then
        print("error: for configure interface *" .. ipif_paramList.ifname .. "* IP Address is mandatory")
        return 1
    end

    if mask == "" and if_section["netmask"] == nil then
        print("error: for configure interface *" .. ipif_paramList.ifname .. "* Netmask is mandatory")
        return 1
    end

    network_ubus_cmd = cmd_ubus_set .. s .. ubus_set_2 .. ubus_set_value
    if count == 1 then
        ubus_set_value_1 = ubus_set_value_1 .. ubus_set_value_list[1]
    elseif count > 1 then
        for j = 1, count - 1 do
            ubus_set_value_1 = ubus_set_value_1 .. ubus_set_value_list[j] .. ","
        end
        ubus_set_value_1 = ubus_set_value_1 .. ubus_set_value_list[count]
    end
    network_ubus_cmd = network_ubus_cmd .. ubus_set_value_1 .. "}}'"
    local res = os.execute(network_ubus_cmd)
    if res ~= 0 then
        print(pm.critical.set_ifconfig)
        return 1
    end
    return 0
end

local function dhcp_handler(if_json, s)
    local res = 1
    local options = ""
    local options_isEmpty = true
    if if_json["ipaddr"] ~= nil then
        options = "\"ipaddr\""
        options_isEmpty = false
    end
    if if_json["netmask"] ~= nil then
        if options_isEmpty then
            options = options .. ", "
        end
        options = "\"netmask\","
        options_isEmpty = false
    end
    if if_json["gateway"] ~= nil then
        if options_isEmpty then
            options = options .. ", "
        end
        options = "\"gateway\","
        options_isEmpty = false
    end
    if if_json["dns"] ~= nil then
        if options_isEmpty then
            options = options .. ", "
        end
        options = "\"dns\","
        options_isEmpty = false
    end
    cmd_ubus_del = cmd_ubus_del .. s .. "\", \"options\":[" .. options .. "]}'"
    if options ~= nil and options ~= "" then
        res = os.execute(cmd_ubus_del)
        if res ~= 0 then
            print(pm.critical.delete_ifconfig)
        end
    end
    cmd_ubus_set = cmd_ubus_set .. s .. "\", \"values\":{\"proto\":\"dhcp\"}}'"
    res = os.execute(cmd_ubus_set)
    if res ~= 0 then
        print(pm.critical.set_ifconfig)
        return 1
    end
    return 0
end

local function ipif_config(if_json)
    local res = ""
    local section = ""
    section = ipif_paramList.ifname

    if ipif_paramList.proto == "static" then
        res = static_handler(if_json, section)
    elseif ipif_paramList.proto == "dhcp" then
        res = dhcp_handler(if_json, section)
    end
    return res
end

local function set_ipif_param()
    local network_jsonInfo = tf.collectJsonTable(cmd_network_ubus)
    if network_jsonInfo == nil or network_jsonInfo["values"] == nil then
        print(pm.critical.config_is_wrong)
        return
    end
    network_jsonInfo = network_jsonInfo["values"]
    getparam()
    if network_jsonInfo[ipif_paramList.ifname] == nil then
        print("error: interface name is wrong")
        return
    end

    if ipif_paramList.ifname == "loopback" then
        print("error: interface \"loopback\" can not be modified")
        return
    end
    local res = ipif_config(network_jsonInfo[ipif_paramList.ifname])
    if res == 0 then
        print(pm.info.successfull)
    end
end

set_ipif_param()
