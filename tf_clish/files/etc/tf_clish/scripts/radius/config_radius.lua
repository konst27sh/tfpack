#!/usr/bin/lua

---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by sheverdin
--- DateTime: 6/6/24 6:25 PM
---

local tf = require "tf_module"
local lfs = require("lfs")
local trf = require "transfer_file_module"

local ubus_set_cmd = "ubus call uci set '{\"config\":\"8021x\", \"type\":"
local ubus_set_cmd_ports_1 = "ubus call uci set '{\"config\":\"8021x\", \"section\":\"lan"
local ubus_set_cmd_ports_2 = "\", \"type\":\"interface\", \"values\":"

local function set_general()
    if arg[3] == nil and arg[3] == "" then
        print("error: value is empty")
        return
    end
    local config_type = arg[1]
    local param = arg[2]
    local value = arg[3]
    if param == "eap_reauth_period" then
        value = tonumber(value)
        if value > 86400 then
            print("error: eap reauth period must be less 86400 seconds")
            return
        end
    end

    local values = "\"values\":{\"" .. param .. "\":\"" .. value .. "\"}}'"
    local cmd = ubus_set_cmd .. "\"" .. config_type .. "\", " .. values
    tf.executeCommand(cmd)
end

local function set_certificate()

    local err = "error"
    local local_file_s = {}
    if trf.transfer_param["local_file"] ~= nil then
        local_file_s = trf.transfer_param["local_file"]
        if trf.transfer_param["local_file"] ~= nil then
            local_file_s = local_file_s["certificate"]
            err = "no_error"
        end
    end
    if err == "error" then
        print("error: transfer module is wrong")
        return "error: transfer module is wrong"
    end

    print()
    if arg[3] == nil then
        print("error: file name is wrong")
        return
    elseif arg[3] == "delete" then
        local cert_name = arg[2]
        local ubus_cmd = "ubus call uci set '{\"config\":\"8021x\", \"type\":\"general\", \"values\":{\"" .. cert_name .. "\":\"\"}}'"
        tf.executeCommand(ubus_cmd)
    else
        local isfilePath = arg[3]:find("[\/:%*%?\"<>|] ")
        if isfilePath then
            print("error: file name must be only word, without special symbols [, \\,  /, :, %, *, %, ?,\", <, >, |,]")
            return
        else
            local file_name = arg[3]
            file_name = trf.check_remoteFilename(file_name)
            local attr = lfs.attributes(local_file_s.path_dir .. file_name)
            if attr ~= nil then
                if attr.mode == "file" then
                    local cert_name = arg[2]
                    local ubus_cmd = "ubus call uci set '{\"config\":\"8021x\", \"type\":\"general\", \"values\":{\"" .. cert_name .. "\":\"" .. local_file_s.path_dir .. file_name .. "\"" .. "}}'"
                    tf.executeCommand(ubus_cmd)
                    print("info: set " .. cert_name .. " successful")
                else
                    print("warning: file mode = " .. attr.mode)
                    return
                end
            else
                print("error: file not found, upload this file first. Use \"tools transfer_file\" command")
                return
            end
        end
    end
end

local function set_server()
    local param = arg[1] .. "_" .. arg[2]
    if arg[3] == nil and arg[3] == "" then
        print("error: value is empty")
        return
    end
    local value = arg[3]
    if arg[2] == "server_port" then
        value = tonumber(value)
        if value > 65535 then
            print("error: port value must be less 65535")
            return
        end
    end
    if arg[2] == "server_shared_secret" then
        if #value > 32 then
            print("error: max length secret word: 32")
            return
        end
    end
    local values = "\"values\":{\"" .. param .. "\":\"" .. value .. "\"}}'"
    local cmd = ubus_set_cmd .. "\"general\", " .. values
    tf.executeCommand(cmd)
end

local function set_ports()
    if arg[3] == nil and arg[3] == "" then
        print("error: value is empty")
        return
    end
    local portRange = arg[2]
    local ports = tf.checkPortRange(portRange)
    local value = arg[3]
    local values = "{\"enable\":\"" .. value .. "\"}}'"
    if ports ~= nil then
        if ports[3] == "once" or ports[3] == "range" then
            local portMin = tonumber(ports[1])
            local portMax = tonumber(ports[2])
            for i = portMin, portMax do
                local cmd = ubus_set_cmd_ports_1 .. i .. ubus_set_cmd_ports_2 .. values
                tf.executeCommand(cmd)
            end
        else
            print("error: port range is wrong")
        end
    else
        print("error: port range is wrong")
    end
end

local function set_log()
    local param = arg[1]
    local value = arg[2]
    local values = "\"values\":{\"" .. param .. "\":\"" .. value .. "\"}}'"
    local cmd = ubus_set_cmd .. "\"logs\", " .. values
    tf.executeCommand(cmd)
end

local general_list =
{
    ["enable"] = set_general,
    ["eap_reauth_period"] = set_general,
    ["ca_cert"] = set_certificate,
    ["server_cert"] = set_certificate,
    ["private_key"] = set_certificate,
}

local serverType_list =
{
    ["server_addr"] = set_server,
    ["server_port"] = set_server,
    ["server_shared_secret"] = set_server,
}

local config_list =
{
    ["general"] = general_list,
    ["auth"] = serverType_list,
    ["acct"] = serverType_list,
    ["ports"] = set_ports,
    ["logger_syslog_level"] = set_log,
}

local function run_config_radius()

    local ubus_radiusConfig_cmd = "ubus -S call uci get '{\"config\":\"8021x\"}'"
    local radiusConfig_json = tf.collectJsonTable(ubus_radiusConfig_cmd)
    if radiusConfig_json == nil then
        print("error: config is wrong")
        return
    end

    local configType_1 = ""
    local configType_2 = ""
    if arg[1] == nil and arg[1] == "" then
        print("error: command 1 is wrong")
        return
    end

    if arg[2] == nil and arg[2] == "" then
        print("error: command 2 is wrong")
        return
    end

    configType_1 = arg[1]
    if config_list[configType_1] == nil then
        print("error: command is wrong")
        return
    end

    local configStruct_1 = config_list[configType_1]
    if type(configStruct_1) == "table" then
        configType_2 = arg[2]
        if configStruct_1[configType_2] == nil then
            print("error: command is wrong")
            return
        else
            configStruct_1[configType_2]()
        end

    elseif type(configStruct_1) == "function" then
        configStruct_1()
    else
        print("error: command is wrong")
        return
    end
end

run_config_radius()
