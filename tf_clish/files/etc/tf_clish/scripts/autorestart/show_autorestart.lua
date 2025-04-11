#!/usr/bin/lua

local tf = require "tf_module"

local cmd_get_ar_status = "ubus call autorestart getStatus"
local cmd_get_ar_config = "ubus call uci get '{\"config\":\"tf_autorestart\"}'"

local ar_enum_status =
{
    status       = "Status:           ",
    test_type    = "Test type:        ",
    speed_kbit   = "speed:            ",
    error_Code   = "Error code:       ",
    reboot_cnt   = "Reboot counts:    ",
    time         = "last reboot time: ",
}

local ar_testType =
{
    "Link",
    "Ping",
    "Speed",
    "Time"
}

local function run_show_autorestart_status()
    local arInfo = tf.collectJsonTable(cmd_get_ar_status)
    if arInfo == nil and arInfo["port"] == nil then
        return "status_error"
    end

    arInfo = arInfo["port"]
    for index, list in pairs(arInfo) do
        print("port " .. index)
        local value = list["status"]
        local str = ar_enum_status["status"]
        print("\t" .. str .. "\t" .. value)

        if value ~= "test disable" and value ~= "port disable" then
            value = list["test_type"]
            str = ar_enum_status["test_type"]
            value = tonumber(value)
            local testType = ""
            if value >= 1 and value <= 4 then
                testType = ar_testType[tonumber(value)]
            elseif value == 0 then
                testType = "test_disable"
            elseif value >= 5 then
                testType = "error: autorestart test type not defined"
            end
            print("\t" .. str .. "\t" .. testType)
            if testType == "Speed" then
                value = list["speed_kbit"]
                str = ar_enum_status["speed_kbit"]
                print("\t" .. str .. "\t" .. value .. " [Kbit/sec]")
            end
            value = list["reboot_cnt"]
            str = ar_enum_status["reboot_cnt"]
            print("\t" .. str .. "\t" .. value)

            value = list["time"]
            str = ar_enum_status["time"]
            print("\t" .. str .. "\t" .. value)
        end
        print("============================================")
    end
end

local function run_show_autorestart_config()
    local arInfo = tf.collectJsonTable(cmd_get_ar_config)
    arInfo = arInfo["values"]
    local portName = tf.portList.portName
    local arList = {}
    for i = 1, tf.portList.maxLanPorts do
        print("\t------------------------------------------")
        local portIndex = portName
        portIndex = portIndex .. i
        arList = arInfo[portIndex]

        print("port " .. " " .. i)
        print("\t" .. "State:\t" .. arList["mode"])

        if arList["mode"] == "ping" then
            if arList["host"] ~= nil and arList["host"] ~= "" then
                print("\t\t\t" .. "host:     \t" .. arList["host"])
            else
                print("\t\t\t" .. "host:     \t" .. "not set")
            end
        elseif arList["mode"] == "speed" then
            if arList["max_speed"] ~= nil and arList["max_speed"] ~= "" then
                print("\t\t\t" .. "max speed:\t" .. arList["max_speed"])
            else
                print("\t\t\t" .. "max speed:\t" .. "not set")
            end
            if arList["min_speed"] ~= nil and arList["min_speed"] ~= "" then
                print("\t\t\t" .. "min speed:\t" .. arList["min_speed"])
            else
                print("\t\t\t" .. "min speed:\t" .. "not set")
            end
        end

        if arList["alarm"] ~= nil and arList["alarm"] ~= "" then

            print("\t" .. "Alarm:\t" .. arList["alarm"])
            if arList["alarm"] == "enable" then

                if arList["timeUp"] ~= nil and arList["timeUp"] ~= "" then
                    print("\t\t\t" .. "timeUp:   \t" .. arList["timeUp"])
                else
                    print("\t\t\t" .. "timeUp:   \t" .. "not set")
                end
                if arList["timeDown"] ~= nil and arList["timeDown"] ~= "" then
                    print("\t\t\t" .. "timeDown: \t" .. arList["timeDown"])
                else
                    print("\t\t\t" .. "timeDown: \t" .. "not set")
                end
            end
        end
    end
end

local ar_cmdList =
{
    ["status"] = { func = run_show_autorestart_status },
    ["config"] = { func = run_show_autorestart_config },
}

local function run_show_autorestart()
    if #arg < 1 then
        print("critical: Incompleted command")
        return
    end

    if arg[1] == nil or arg[1] == "" then
        print("critical: Command not valid")
        return
    end

    local ar_show_status = arg[1]
    if ar_cmdList[ar_show_status] == nil then
        print("critical: Command not valid")
        return
    end

    local ar_show_res = ar_cmdList[ar_show_status].func()
    if ar_show_res == "status_error" then
        print("error: can not display autorestart status")
    elseif ar_show_res == "status_error" then
        print("error: can not display autorestart configuration")
    end
end

run_show_autorestart()


