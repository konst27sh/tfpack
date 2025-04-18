#!/usr/bin/lua

---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by sheverdin.
--- DateTime: 7/23/24 4:56 PM
---

local tf = require "tf_module"
local i2c = require "i2c_module"

local cmd_show_input_config = "ubus call uci get '{\"config\":\"tfortis_io\"}'"

local sensorsEnum =
{
    ["0"] = "short",
    ["1"] = "open",
}

local shtEnum =
{
    ["0"] = "no",
    ["1"] = "yes",
}

local function show_io_config(sensor_name)
    local json_IO_info = tf.collectJsonTable(cmd_show_input_config)
    local io_info_all = json_IO_info.values
    local io_info = {}
    if sensor_name == "output" then
        io_info = io_info_all.relay
        print("Relay state: " .. io_info["state"])
    elseif sensor_name == "input" then
        io_info = io_info_all.tamper
        print("------------------------------------------------")
        print(i2c.sensorList[1].name_str)
        print(i2c.sensorList[1].msg.state .. io_info["state"])
        print(i2c.sensorList[1].msg.alarm_state .. io_info["alarm_state"])
        io_info = io_info_all.sensor1
        print("------------------------------------------------")
        print(i2c.sensorList[2].name_str)
        print(i2c.sensorList[2].msg.state .. io_info["state"])
        print(i2c.sensorList[2].msg.alarm_state .. io_info["alarm_state"])
        io_info = io_info_all.sensor2
        print("------------------------------------------------")
        print(i2c.sensorList[3].name)
        print(i2c.sensorList[3].msg.state .. io_info["state"])
        print(i2c.sensorList[3].msg.alarm_state .. io_info["alarm_state"])
    end
end

local function show_io_status(sensor_name)
    local name = ""
    local ubus_cmd = "ubus call tf_hwsys  getCategory '{\"name\":\"sensors\"}'"
    local jsonSensorInfo = tf.collectJsonTable(ubus_cmd)
    if jsonSensorInfo == nil and jsonSensorInfo == "" then
        print("error: sensors data")
        return
    end
    local res = ""
    if sensor_name == "output" then
        name = i2c.sensorList[4].name
        print(name)
        res = tostring(jsonSensorInfo[name])
        print(i2c.sensorList[4].msg.state .. sensorsEnum[res])
    else
        for i = 1, 3 do
            name = i2c.sensorList[i].name
            print(i2c.sensorList[i].name_str)
            res = tostring(jsonSensorInfo[name])
            print(i2c.sensorList[i].msg.state .. sensorsEnum[res])
        end

        for j = 5, 8 do
            local connection_flag = "no"
            name = i2c.sensorList[j].name
            res = tostring(jsonSensorInfo[name])
            if name == "sensorConnected" then
                print(i2c.sensorList[j].name_str)
                print("\t" .. shtEnum[res])
                connection_flag = shtEnum[res]
            elseif name == "SHT_TYPE" then
                if connection_flag == "yes" then
                    print(i2c.sensorList[j].name_str)
                    print(i2c.sensorList[j].msg.state .. res)
                end
            elseif name == "sensorTemperature" then
                if connection_flag == "yes" then
                    print(i2c.sensorList[j].name_str)
                    print("\t" .. res)
                end
            elseif name == "sensorHumidity" then
                if connection_flag == "yes" then
                    print(i2c.sensorList[j].name_str)
                    print("\t" .. res)
                end
            end
        end
    end
end

local cmd_show_ioList =
{
    config = { cmd = "config", func = show_io_config },
    status = { cmd = "status", func = show_io_status },
}

local function run_show_io()
    local sensor_name   = arg[3]
    local cmd           = arg[2]
    local cmd_list      = cmd_show_ioList[cmd]
    cmd_list.func(sensor_name)
end

local function run_config_io()
    local sensor_name = arg[2]
    local param       = arg[3]
    local value       = arg[4]
    local str_cmd = "uci set tfortis_io." .. sensor_name .. "." .. param .. "=" .. value
    tf.executeCommand(str_cmd)
end

local cmd_ioList =
{
    show   = { cmd = "show",   func = run_show_io },
    config = { cmd = "config", func = run_config_io },
}

local function run_main_io()
    local cmd = arg[1]
    local cmd_list = cmd_ioList[cmd]
    cmd_list.func()
end

run_main_io()

