#!/usr/bin/lua

local hash_cmd = "ubus call uci get '{\"config\":\"rpcd\"}'"

local tf = require "tf_module"
local configTable = {}

local ifname = ""
local portNum = ""


local function sendLog(str)
    local file = io.open("/tmp/setConfig.txt", "w")
    if file then
        file:write(str)
        file:close()
    else
        print("Can not read/write file")
    end
end

local function set_ip()
    local cmd = "/etc/tf_clish/scripts/ipif/set_ipif.lua ifname " .. ifname .. " proto static ip " .. configTable["IPADDRESS"]
    --sendLog("set ip cmd = " .. cmd)
    tf.executeCommand(cmd)
    return "config_ok"
end

local function set_mask()
    local cmd = "/etc/tf_clish/scripts/ipif/set_ipif.lua ifname " .. ifname .. " proto static netmask " .. configTable["NETMASK"]
    tf.executeCommand(cmd)
    return "config_ok"
end

local function set_gate()
    local cmd = "/etc/tf_clish/scripts/ipif/set_ipif.lua ifname " .. ifname .. " proto static gateway " .. configTable["GATEWAY"]
    tf.executeCommand(cmd)
    return "config_ok"
end

local function set_sysName()
    local cmd = "/etc/tf_clish/scripts/description/set_description.lua description " .. configTable["SYSTEM_NAME"]
    tf.executeCommand(cmd)
    return "config_ok"
end

local function set_location()
    local cmd = "/etc/tf_clish/scripts/description/set_description.lua location " .. configTable["SYSTEM_LOCATION"]
    tf.executeCommand(cmd)
    return "config_ok"
end

local function set_snmpState()
    local cmd = ""
    if configTable["SNMP_STATE"] == "1" then
        cmd = "ubus call uci set '{\"config\":\"snmpd\", \"type\":\"general\",\"values\":{\"enabled\":\"1\"}}'"
    end
    tf.executeCommand(cmd)
    return "config_ok"
end

local function set_lldpState()
    return "config_ok"
end

local function set_portState()
    local str = "PORT" .. portNum .. "_STATE"
    local stateValue = configTable[str]
    if stateValue == "1" then
        stateValue = "enable"
    elseif stateValue == "0" then
        stateValue = "disable"
    end
    local cmd = "/etc/tf_clish/scripts/port/config_ports_poe.lua state " .. portNum .. " " .. stateValue
    tf.executeCommand(cmd)
    return "config_ok"
end

local function set_poeState()
    local str = "PORT" .. portNum .. "_POE"
    local stateValue = configTable[str]
    if stateValue == "257" then
        stateValue = "enable"
    elseif stateValue == "0" then
        stateValue = "disable"
    end
    local cmd = "/etc/tf_clish/scripts/port/config_ports_poe.lua poe " .. portNum .. " " .. stateValue
    tf.executeCommand(cmd)
    return "config_ok"
end

local function set_reboot()
    if configTable["REBOOT_ALL"] == "1" then
        return "config_reboot"
    else
        return "config_error"
    end
end

local function set_default()
    if configTable["DEFAULT_ALL"] == "1" then
        return "config_default"
    else
        return "config_error"
    end
end

local keys_configTable =
{
    ["IPADDRESS"]       = set_ip,
    ["NETMASK"]         = set_mask,
    ["GATEWAY"]         = set_gate,
    ["SYSTEM_NAME"]     = set_sysName,
    ["SYSTEM_LOCATION"] = set_location,
    ["SNMP_STATE"]      = set_snmpState,
    ["LLDP_STATE"]      = set_lldpState,
    ["PORT_STATE"]      = set_portState,
    ["PORT_POE"]        = set_poeState,
    ["REBOOT_ALL"]      = set_reboot,
    ["DEFAULT_ALL"]     = set_default
}

local function check_valid_hash()
    local res = "hash_error"
    local userdata = tf.collectJsonTable(hash_cmd)
    userdata = userdata["values"]
    if arg[1] ~= nil and arg[1] ~= "" then
        for _, user in pairs(userdata) do
            if user[".type"] == "login" then
                if user["hash"] ~= nil then
                    if user["hash"] == arg[1] then
                        res = "hash_ok"
                    end
                else
                    -- print("user name = " .. user["username"] .. " NOT HAVE HASH")
                end
            end
        end
    else
        res = "hash_not_valid"
    end
    return res
end

local function getSettingTable()
    local res = "msg_ok"
    if arg[2] ~= nil and arg[2] ~= "" then
        local parts = {}
        for part in string.gmatch(arg[2], "#([^#]+)") do
            table.insert(parts, part)
        end
        for _, part in ipairs(parts) do
            local start_index = string.find(part, "%[") + 1
            local end_index = string.find(part, "%]") - 1
            local key = string.sub(part, 1, start_index - 2)
            key = string.gsub(key, "=", "")
            local value = string.sub(part, start_index, end_index)
            configTable[key] = value
        end
    else
        res = "msg_error"
    end
    return res
end

local function setConfig()
    local res = "config_ok"
    if configTable ~= nil then
        for key, value in pairs(configTable) do
            local num = key:match("%d+")
            key = key:gsub("%d+", "")
            if keys_configTable[key] ~= nil then
                if num ~= nil then
                    portNum = num
                else
                    if key == "IPADDRESS" or
                        key == "NETMASK" or
                        key == "GATEWAY" then
                        if arg[3] ~= nil and arg[3] ~= "" then
                            ifname = arg[3]
                        end
                    end
                end
                res = keys_configTable[key]()
            end
        end
    else
        res = "config_error"
    end
    return res
end

local function run_hash_test()
    local hash_errorCode = "hash_error"
    local msg_errorCode = "msg_error"
    local config_error = ""
    hash_errorCode = check_valid_hash()
    if hash_errorCode == "hash_ok" then
        msg_errorCode = getSettingTable()
        if msg_errorCode == "msg_ok" then
            config_error = setConfig()
            print(config_error)
        else
            print(msg_errorCode)
        end
    else
        print(hash_errorCode)
    end
end

run_hash_test()

