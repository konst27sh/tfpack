---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by sheverdin.
--- DateTime: 3/11/24 4:58 PM
---

local snmp_base = {}

local mib_0 =
{
    ["1.3.6.1.4.1.42019"] =
    {
        name = "forttelecomMIB",
        next_oid = "1.3.6.1.4.1.42019.3.2.1.2.1.1.1"
    }
}

local mib_1 =
{
    ["1.3.6.1.4.1.42019.3"] =
    {
        name = "switch",
        next_oid = "1.3.6.1.4.1.42019.3.2.1.2.1.1.1"
    }
}

local mib_2 =
{
    ["1.3.6.1.4.1.42019.3.2"] =
    {
        name = "psw",
        next_oid = "1.3.6.1.4.1.42019.3.2.1.2.1.1.1"
    }
}

local mib_3 =
{
    ["1.3.6.1.4.1.42019.3.2.1"] =
    {
        name = "configPSW",
        next_oid = "1.3.6.1.4.1.42019.3.2.1.2.1.1.1"
    },
    ["1.3.6.1.4.1.42019.3.2.2"] =
    {
        name = "statusPSW",
        next_oid = "1.3.6.1.4.1.42019.3.2.2.1.1"
    }
}

local mib_4 =
{
    ["1.3.6.1.4.1.42019.3.2.1.2"] =
    {
        name = "autoRestart",
        next_oid = "1.3.6.1.4.1.42019.3.2.1.2.1.1.1"
    },
    ["1.3.6.1.4.1.42019.3.2.1.3"] =
    {
        name = "portPoe",
        next_oid = "1.3.6.1.4.1.42019.3.2.1.3.1.1.1"
    },
    ["1.3.6.1.4.1.42019.3.2.1.4"] =
    {
        name = "outStatePSW",
        next_oid = "1.3.6.1.4.1.42019.3.2.1.4.1"
    },
    ["1.3.6.1.4.1.42019.3.2.2.1"] =
    {
        name = "upsStatus",
        next_oid = "1.3.6.1.4.1.42019.3.2.2.1.1"
    },
    ["1.3.6.1.4.1.42019.3.2.2.2"] =
    {
        name = "inputStatus",
        next_oid = "1.3.6.1.4.1.42019.3.2.2.2.1.1.1"
    },
    ["1.3.6.1.4.1.42019.3.2.2.3"] =
    {
        name = "fwStatus",
        next_oid = "1.3.6.1.4.1.42019.3.2.2.3.1"
    },
    ["1.3.6.1.4.1.42019.3.2.2.5"] =
    {
        name = "poeStatus",
        next_oid = "1.3.6.1.4.1.42019.3.2.2.5.1.1.1"
    },
    ["1.3.6.1.4.1.42019.3.2.2.7"] =
    {
        name = "sfpStatus",
        next_oid = "1.3.6.1.4.1.42019.3.2.2.7.1.1.1"
    },
    ["1.3.6.1.4.1.42019.3.2.2.8"] =
    {
        name = "sensorEntry",
        next_oid = "1.3.6.1.4.1.42019.3.2.2.8.1"
    },
}

snmp_base.main =
{
    mib_0,
    mib_1,
    mib_2,
    mib_3,
    mib_4
}

return snmp_base

