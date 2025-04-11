#!/usr/bin/env lua

local tf = require "tf_module"

local bridge_module = {}

bridge_module.if_enum = {
    device          = "\tdevice:            \t",
    proto           = "\tproto:             \t",
    ipaddr          = "\tip address:        \t",
    netmask         = "\tmask:              \t",
    dns             = "\tdns:               \t",
    status          = "\tstatus:            \t",
    uptime          = "\tuptime:            \t",
    dns_server      = "\t\tdns server:      \t",
    dns_search      = "\t\tdns search:      \t",
    target          = "\t\ttarget:          \t",
    nexthope        = "\t\tnext hope:       \t",
    source          = "\t\tsource:          \t",
    vlan_id         = "Vlan ID:             \t\t",
    mac_addr        = "\nMAC Address:       \t\t"
}

bridge_module.board_param = {
    model           = "\tModel:             \t",
    hostname        = "\tHost name:         \t",
    version         = "\tFirmware Version:  \t",
}

bridge_module.time_param = {
    time_and_date   = "\tSystem Time:       \t\t",
    uptime          = "\tSystem Uptime:     \t\t",
    zonename        = "\tTime zone:         \t\t"
}

bridge_module.system_param = {
    description     = "\tDescription:       \t",
    contacts        = "\tContact:           \t",
    location        = "\tLocation:          \t",
}

local mdb_param = {
    ["grp"]     = "\tGroup:       \t",
    ["state"]   = "\tState:       \t",
    ["vid"]     = "\tVlan id:     \t",
    ["dev"]     = "\tDevice:      \t",
    ["port"]    = "\tPort:        \t",
    ["flags"]   = "\tFlags:       \t",
    ["index"]   = "\tIndex:       \t",
}

bridge_module.mdb_json = {
    ["router"] = { name = "Router:  ", struct = nil },
    ["mdb"]    = { name = "MDB:     ", struct = mdb_param }
}

bridge_module.fdb_params = {
    ["mac"]     = "\tMAC address: \t",
    ["ifname"]  = "\tInterface:   \t",
    ["vlan"]    = "\tVLAN id:     \t",
    --["flags"]   =\t "Flag:        \t",
    ["master"]  = "\tMaster:      \t",
    ["state"]   = "\tState:       \t",
}

local function get_mdb(isPrint)
    local mdb_cmd = "/usr/sbin/bridge -j mdb show"
    local igmp_status_json = tf.executeCommand(mdb_cmd)
    if isPrint == "yes" then
        print(igmp_status_json)
    end
    return igmp_status_json

end

local function get_fdb(isPrint)
    local fdb_cmd = "bridge -j fdb show dynamic"
    local fdb_table = {}
    local mac_status_json = tf.executeCommand(fdb_cmd)
    if isPrint == "yes" then
        print(mac_status_json)
    end
    return mac_status_json
end

function bridge_module.br_getIflist(cmd)
    local ifList = {}
    local iflist_res = ""
    local if_arr = {}
    local interfaceList = tf.collectJsonTable(cmd)
    if interfaceList == nil or interfaceList["interface"] == nil then
        return ifList, 1
    else
        interfaceList = interfaceList["interface"]
    end
    for _, if_list in pairs(interfaceList) do
        local res = ""
        local dev = {}
        if if_list["device"] ~= nil then
            dev = if_list["device"]
            res = string.find(dev, "switch")
        end
        if res ~= nil and res == 1 then
            table.insert(if_arr, if_list["interface"])
            iflist_res = "if_list_ok"
        end
    end
    if iflist_res ~= "if_list_ok" then
        return if_arr, 2
    end
    return if_arr, 0
end

bridge_module.options =
{
    ["mdb"] = get_mdb,
    ["fdb"] = get_fdb
}



--if arg[1] ~= nil then
--    local isPrint = "no"
--    if bridge_module.options[arg[1]] ~= nil then
--        if arg[2] ~= nil and arg[2] == "print" then
--            isPrint = "yes"
--        end
--        bridge_module.options[arg[1]](isPrint)
--    end
--end

return bridge_module
