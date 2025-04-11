#!/usr/bin/lua

local vlan_utils = {}
local tf = require "tf_module"

local tagget_enum = {
    tagged      = "T",
    untagged    = "U"
}

local vlan_str = {
    id = "ID:             \t",
    num = "VID:  ",
    device = "\t Device:     \t",
    state = "\t State:       \t",
    descr = "\t Description: \t",
    ports = "\t Ports:       \t",
}

local function getPortType(portStr)
    local port = {}
    local portNumber = 0
    local state = "untagged"

    local portStr_1 = tostring(portStr)
    if string.find(portStr, ":t") then
        local str, num = portStr_1:match("(%w+)(%d+):(%a+)")
        portNumber = tonumber(num)
        state = "tagged"
    else
        local str, num = portStr_1:match("(%a+)(%d+)")
        portNumber = tonumber(num)
    end
    port.state      = state
    port.portNumber = portNumber
    return port
end

local function printVlanPortsParm(portList)
    for num, type in pairs(portList) do
        --print("\t\t\t" .. port.portNumber .. "  " .. tagget_enum[port.state])
        print("\t\t\t" .. num .. "\t" .. type)
    end
end

function vlan_utils.setVlanParam(param)
    local data = ""
    if param ~= nil then
        data = param
        return data
    else
        return "not_set"
    end
end

function vlan_utils.setPorts(ports)
    local portsList = {}
    if ports ~= nil then
        for i, p in pairs(ports) do
            local port = {}
            port.state = ""
            port.portNumber = 0
            port = getPortType(p)
            portsList[port.portNumber] = tagget_enum[port.state]
        end
    else
        portsList = nil
    end
    return portsList
end

function vlan_utils.printVlanParm(vlan)
    if (vlan.mngtVlan == "yes") then
        if type(vlan.ifname) == "table" then
            print(vlan_str.num .. vlan.num .. " >>> Management interface: ")
            for i, if_name in pairs(vlan.ifname) do
                print("\t\t\t\t" .. if_name)
            end
        else
            print(vlan_str.num .. vlan.num .. " >>> Management interface: " .. vlan.ifname)
        end
    else
        print(vlan_str.num .. vlan.num)
    end
    if (vlan.state ~= nil) then
        print(vlan_str.state .. vlan.state)
    else
        print("vlan_str.state" .. "------ not set ------ ")
    end

    if (vlan.device ~= nil) then
        print(vlan_str.device .. vlan.device)
    else
        print(vlan_str.device .. "------ not set ------ ")
    end

    if (vlan.descr ~= nil) then
        print(vlan_str.descr .. vlan.descr)
    else
        print(vlan_str.descr .. "------ not set ------ ")
    end
    if (vlan.ports ~= nil) then
        print(vlan_str.ports)
        printVlanPortsParm(vlan.ports)
    else
        print(vlan_str.ports .. "------ not set ------ ")
    end
end

function vlan_utils.saveVlan(vlanList, action)
    local saveVlan = false
    local busyList = {}
    if vlanList.ports == nil then
        if action == "edit" then
            print("info: no ports assigned on Vlan *" .. vlanList.num .. "*, use command add")
            return busyList, saveVlan
        end
        busyList = {}
    else
        local current_vlanPortList = {}
        for num, state in pairs(vlanList.ports) do
            --print("num = " .. num .. " state = " .. state)
            --print("add to list")
            current_vlanPortList[num] = state
        end
        busyList = current_vlanPortList
    end
    saveVlan = true
    return busyList, saveVlan
end

function vlan_utils.uciSetVlanParam(vlanArr, busy_vlanPortList)
    --print("====================  uciSetVlanParam ==================== ")
    local cmd_del_port = ""
    local add_untagged_cmd = ""
    for vlan_num, portlist in pairs(busy_vlanPortList) do
        local vlanList = vlanArr[tostring(vlan_num)]

        if vlanList.ports ~= nil then
            cmd_del_port = "uci del network." .. vlanList.id .. ".ports"
            tf.executeCommand(cmd_del_port)
        end
        for num, state in pairs(portlist) do
            --print("num = " .. num .. " state = " .. state)
            if state == "U" then
                add_untagged_cmd = "uci add_list network." .. vlanList.id .. ".ports=lan" .. num
            elseif state == "T" then
                add_untagged_cmd = "uci add_list network." .. vlanList.id .. ".ports=lan" .. num .. ":t"
            end
            --print("add_untagged_cmd = " .. add_untagged_cmd)
            local res = tf.executeCommand(add_untagged_cmd)
        end
    end
    return "vlan_ok"
end

function vlan_utils.check_isDeleted(port_num, id, section)
    if tf.ubusList == nil or tf.ubusList.cmd_ubus == nil or tf.ubusList.cmd_changes == nil then
        return "error_1"
    end

    local changes_cmd = tf.ubusList.cmd_ubus .. tf.ubusList.cmd_changes
    local changes_info = tf.collectJsonTable(changes_cmd)
    if changes_info == nil or changes_info["changes"] == nil then
        return "error_2"
    end
    changes_info = changes_info["changes"]
    if changes_info[section] == nil then
        return "no_changes"
    end

end

return vlan_utils







