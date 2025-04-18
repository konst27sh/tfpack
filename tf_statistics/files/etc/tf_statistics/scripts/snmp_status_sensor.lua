---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by sheverdin.
--- DateTime: 3/11/24 4:58 PM
---

local snmp_status_sensor = {}

local i2c = require "i2c_module"
local tf = require "tf_module"

local shtEnum =
{
    ["0"] = "2",
    ["1"] = "1",
}

local function i2c_handler(mibObj, index)
    local jsonInfo
    local res = ""
    local name = ""
    local status = "OK"
    if mibObj.name == "inputState" or
        mibObj.name == "inputType" or
        mibObj.name == "inputAlarm" or
        mibObj.name == "inputIndex"
    then
        if tonumber(index) > 4 or tonumber(index) <= 0 then
            status = "over_index"
            res = nil
        else
            local sList = i2c.sensorList[tonumber(index)]
            name = sList.name
        end
    elseif mibObj.name == "relay" then
        if tonumber(index) > 1 then
            status = "over_index"
        else
            name = "relay"
        end
    else

        name = mibObj.name
    end

    if status == "OK" then
        if mibObj.name == "inputIndex" then
            res = index
        elseif mibObj.name == "inputType" or
                mibObj.name == "inputAlarm" or
                mibObj.name == "inputState" or
                mibObj.name == "relay" then
            if mibObj.name == "inputType" then
                res = tf.executeCommand("uci -q get tfortis_io." .. name .. ".type")
                if mibObj.enum ~= nil then
                    res = mibObj.enum[res]
                end
            elseif mibObj.name == "inputState" then
                jsonInfo = tf.getUbusDataByName(name)
                if jsonInfo == nil then
                    res = nil
                else
                    res = jsonInfo[name]
                    if (mibObj.isZero == "isZero") then
                        res = shtEnum[res]
                    end
                end
            elseif mibObj.name == "inputAlarm" then
                res = tf.executeCommand("uci -q get tfortis_io." .. name .. ".alarm_state")
                if (mibObj.isEnum == "yes") then
                    res = mibObj.enum[res]
                end
            elseif mibObj.name == "relay" then
                res = tf.executeCommand("uci -q get tfortis_io.relay.state")
                if (mibObj.isEnum == "yes") then
                    res = mibObj.enum[res]
                end
            end
        else
            jsonInfo = tf.getUbusDataByName(mibObj.name)
            if jsonInfo == nil and jsonInfo == "" then
                res = nil
            else
                res = jsonInfo[mibObj.name]
                if (mibObj.isZero == "isZero") then
                    res = shtEnum[res]
                end
            end
        end
    end
    return res, status
end

snmp_status_sensor.main =
{
    ["1.3.6.1.4.1.42019.3.2.2.2.1.1.1"] =
    {
        valueType = "INTEGER",
        nodetype = "column",
        oidLength = "7",
        ubusType = i2c_handler,
        i2c_addr = "0",
        isZero = "no",
        name = "inputIndex",
        next_oid = "1.3.6.1.4.1.42019.3.2.2.2.1.1.2"
    },
    ["1.3.6.1.4.1.42019.3.2.2.2.1.1.2"] =
    {
        nodetype = "column",
        oidLength = "7",
        isEnum = "yes",
        enum  = {
            ["build-in\n"] = "1",
            ["plc\n"]      = "2",
        },
        i2c_addr = "0",
        valueType = "INTEGER",
        ubusType = i2c_handler,
        name = "inputType",
        isZero = "no",
        next_oid = "1.3.6.1.4.1.42019.3.2.2.2.1.1.3"
    },
    ["1.3.6.1.4.1.42019.3.2.2.2.1.1.3"] =
    {
        nodetype = "column",
        oidLength = "7",
        isEnum = "yes",
        enum  = {
            ["short\n"] = "1",
            ["open\n"] = "2",
        },
        i2c_addr = "0",
        valueType = "INTEGER",
        ubusType = i2c_handler,
        name = "inputState",
        isZero = "isZero",
        next_oid = "1.3.6.1.4.1.42019.3.2.2.2.1.1.4"
    },
    ["1.3.6.1.4.1.42019.3.2.2.2.1.1.4"] =
    {
        nodetype = "column",
        oidLength = "7",
        isEnum = "yes",
        enum  = {
            ["any\n"] = "3",
            ["short\n"] = "1",
            ["open\n"] = "2",
        },
        i2c_addr = "0",
        valueType = "INTEGER",
        ubusType = i2c_handler,
        name = "inputAlarm",
        isZero = "no",
        next_oid = "1.3.6.1.4.1.42019.3.2.2.3.1"
    },
}

return snmp_status_sensor

