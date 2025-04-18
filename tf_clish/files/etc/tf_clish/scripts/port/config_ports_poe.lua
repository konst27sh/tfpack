#!/usr/bin/lua

---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by sheverdin.
--- DateTime: 6/3/24 4:52 PM
---
local tf = require "tf_module"

local speedList =
{
    "auto",
    "1000 duplex full",
    "100 duplex full",
    "100 duplex half",
    "10 duplex full",
    "10 duplex half",
}

local function set_parm(port)
    local cmd = "uci set "
    local str1 = "port.lan"
    local value = arg[3]
    if arg[1] == "speed" then
        value = tonumber(value)
        value = "'" .. speedList[value] .. "'"
    end
    str1 = str1 .. port
    cmd = cmd .. str1 .. "." .. arg[1] .. "=" .. value
    tf.executeCommand(cmd)
end

local function set_port_config()
    local portRange = tf.checkPortRange(arg[2])
    local cmd = "ubus call uci get '{\"config\":\"port\"}'"
    local portInfo = tf.collectJsonTable(cmd)
    local portsList = portInfo.values
    local count = 0
    for _, list in pairs(portsList) do
        if type(list) == "table" then
            if list.poe ~= nil then
                count = count + 1
            end
        end
    end

    if portRange[3] == "all" then
        for index = 1, count do
            set_parm(index)
        end
    elseif portRange[3] == "range" then
        for index = portRange[1], portRange[2] do
            set_parm(index)
        end
    elseif portRange[3] == "once" then
        set_parm(portRange[1])
    else
        print("port range not corrected")
    end
end

set_port_config()
