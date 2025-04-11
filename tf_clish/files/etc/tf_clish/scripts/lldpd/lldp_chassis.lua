

local lldpd = require "lldp_utils"
local chassis_module = {}

function chassis_module.parsingChassis(chassis_s, flag)
    --print(chassis_s[utils.chassis_e.name].value .. ":")

    if flag then
        print("***************************************************")
    end

    local port_name = ""
    if chassis_s.name ~= nil then
        port_name = chassis_s.name
    end

    local id_type  = lldpd.chassis_e.id_t.type
    local id_value = lldpd.chassis_e.id_t.value
    if chassis_s.chassis ~= nil then
        local chassis_table = chassis_s.chassis
        if port_name ~= nil and port_name ~= "" then
            port_name = string.gsub(port_name, "lan(%d+)", "port %1")
            print("Local Interface: \t" .. port_name .. "\n")
        end
        if flag then
            print("Discovered chassis: ")
        end
        for _, chassis_struct in pairs(chassis_table) do
            local id_arr = chassis_struct[lldpd.chassis_e.id]

            if chassis_struct[lldpd.chassis_e.id] ~= nil then
                for _, id_table in pairs(chassis_struct[lldpd.chassis_e.id]) do
                    print("\tid: " .. "\t\t" .. id_table[id_type] .. "\t" .. (id_table[id_value]))
                end
            end

            if chassis_struct[lldpd.chassis_e.descr] ~= nil then
                for _, struct in pairs(chassis_struct[lldpd.chassis_e.descr]) do
                    print("\tdescription: " .. "\t" .. tostring(struct["value"]))
                end
            end

            local capability_type    = lldpd.chassis_e.capability_t.type
            local capability_enabled = lldpd.chassis_e.capability_t.enabled
            print("\t" .. lldpd.chassis_e.capability .. ":")

            if chassis_struct[lldpd.chassis_e.capability] ~= nil then
                for _, struct in pairs(chassis_struct[lldpd.chassis_e.capability]) do
                    if (struct[capability_enabled] == true) then
                        print("\t\t" .. struct[capability_type])
                    end
                end
            end

            print("\tmgmt ip: ")
            if chassis_struct[lldpd.chassis_e.mgmt_ip] ~= nil then
                for _, struct in pairs(chassis_struct[lldpd.chassis_e.mgmt_ip]) do
                    print("\t\t" .. struct["value"])
                end
            end

            print("\tmgmt iface: ")
            if chassis_struct[lldpd.chassis_e.mgmt_iface] ~= nil then
                for _, struct in pairs(chassis_struct[lldpd.chassis_e.mgmt_iface]) do
                    print("\t\t" .. tostring(struct["value"]))
                end
            end
        end
    end

    if chassis_s.port ~= nil then
        local port_table = chassis_s.port
        print("\nDiscovered port:")
        for _, detected_port in pairs(port_table) do
            if detected_port[lldpd.detected_port.id] ~= nil then
                for _, id_table in pairs(detected_port[lldpd.detected_port.id]) do
                    print("\tid: " .. "\t" .. id_table[id_type] .. "\t" .. (id_table[id_value]))
                end
            else
                --print("detected_port[lldpd.detected_port.id] == nil")
            end

            if detected_port[lldpd.detected_port.descr] ~= nil then
                for _, descr_struct in pairs(detected_port[lldpd.detected_port.descr]) do
                    local detected_port_name = tostring(descr_struct["value"])
                    detected_port_name = string.gsub(detected_port_name, "lan(%d+)", "%1")
                    print("\tport: " .. "\t" .. detected_port_name)
                end
            else
                --print("detected_port[lldpd.detected_port.descr] == nil")
            end

            if detected_port[lldpd.detected_port.ttl] ~= nil then
                for _, ttl_struct in pairs(detected_port[lldpd.detected_port.ttl]) do
                    print("\tTTl: " .. "\t" .. tostring(ttl_struct["value"]))
                end
            else
                --print("detected_port[lldpd.detected_port.ttl] == nil")
            end
        end
    end
end

return chassis_module

