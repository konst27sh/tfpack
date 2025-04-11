#!/usr/bin/lua
--
-- Created by sheverdin.
-- DateTime: 2/14/24 12:06 PM
--

package.path = "/etc/tf_clish/scripts/stp/?.lua;" .. package.path
local tf  = require "tf_module"
local uci = require("luci.model.uci").cursor()
local stp = require "stp_module"

local function config_portPriority(port)
    local stp_portPriority_err = 1
    local value = tonumber(arg[3])
    if value >= 0 and value <= 15 then
        uci:set("mstpd", port, "treeprio", tostring(arg[3]))
        stp_portPriority_err = 0
    else
        print("error: Priority value not corrected")
    end
    return stp_portPriority_err
end

local function config_portPathcost(port)
    local stp_portPathcost_err = 1
    local value = tonumber(arg[3])
    if value >= 0 or value <= 200000000 then
        uci:set("mstpd", port, "pathcost", tostring(arg[3]))
        stp_portPathcost_err = 0
    else
        print("error: Path cost value maust be in range  0 - 200000000")
    end
    return stp_portPathcost_err
end

local function config_portAdmin_edge(port)
    local stp_portAdmin_edge_err = 1
    if arg[3] == "yes" or arg[3] == "no" then
        uci:set("mstpd", port, "adminedge", tostring(arg[3]))
        stp_portAdmin_edge_err = 0
    else
        print("error: Admin edge value not corrected")
    end
    return stp_portAdmin_edge_err
end

local function config_port_Auto_edge(port)
    local stp_port_Auto_edge_err = 1
    if arg[3] == "yes" or arg[3] == "no" then
        uci:set("mstpd", port, "autoedge", tostring(arg[3]))
        stp_port_Auto_edge_err = 0
    else
        print("error: Auto_edge value not corrected")
    end
    return stp_port_Auto_edge_err
end

local function config_portBpdu_filter(port)
    local stp_portBpdu_filter_err = 1
    if arg[3] == "yes" or arg[3] == "no" then
        uci:set("mstpd", port, "bpdufilter", tostring(arg[3]))
        stp_portBpdu_filter_err = 0
    else
        print("error: BPDU filter value not corrected")
    end
    return stp_portBpdu_filter_err
end

local function config_portBpdu_guard(port)
    local stp_portBpdu_guard_err = 1
    if arg[3] == "yes" or arg[3] == "no" then
        uci:set("mstpd", port, "bpduguard", tostring(arg[3]))
        stp_portBpdu_guard_err = 0
    else
        print("error: BPDU filter value not corrected")
    end
    return stp_portBpdu_guard_err
end

local cmdPortList =
{
    {cmd = "treeprio",      func = config_portPriority },
    {cmd = "pathcost",      func = config_portPathcost },
    {cmd = "adminedge",     func = config_portAdmin_edge },
    {cmd = "autoedge",      func = config_port_Auto_edge },
    {cmd = "bpdufilter",    func = config_portBpdu_filter },
    {cmd = "bpduguard",     func = config_portBpdu_guard },
}

local function run_config_stp_port()
    local portRange = tf.checkPortRange(arg[1])
    if portRange ~= nil then
        local minPort = tonumber(portRange[1])
        local maxPort = tonumber(portRange[2])
        local netConfig = tf.collectJsonTable(stp.cmdList.get_netConfig)
        if netConfig == nil or netConfig["values"] == nil then
            print("critical: network config is wrong")
        end
        local portList = tf.getPortList(netConfig["values"], "switch", "ports")
        for _, portStr in pairs(portList) do
            local port, _ = tf.getPort(portStr)
            local portNum = tonumber(port)
            if (portNum >= minPort and portNum <= maxPort) then
                for _, value in pairs(cmdPortList) do
                    if arg[2] == value.cmd then
                        local stp_port_res = value.func(portStr)
                        if stp_port_res == 0 then
                            print("info: " .. arg[2] .. " successful!")
                        end
                    end
                end
            end
        end
    else
        io.write("error: stp port range is wrong\n")
    end
end

run_config_stp_port()
