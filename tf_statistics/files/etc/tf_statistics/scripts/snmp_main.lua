#!/usr/bin/lua

---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by sheverdin.
--- DateTime: 4/26/24 4:11 PM
---
---
package.path = "/etc/tf_statistics/scripts/?.lua;" .. package.path

local tf        = require "tf_module"
local snmp      = require "tf_snmp_module"
local log       = require "tf_log"

local requestType = arg[1]
local inputOid    = arg[2]

local setParam =
{
    valueType   = "",
    value       = ""
}

local function send_Get_result(oid, mibObj, res)
    print(oid)
    if mibObj ~= nil then
        if (mibObj.valueType == "DisplayString") then
            print("String")
            res = string.gsub(res, "%s+$", "")
            --print(res)
        else
            print(mibObj.valueType)
            print(res)
        end
    else
        print("OID")
        print("1.3.6.1.4.1.8072.3.2.10")
    end
end

local statusList = {
    "ok",
    "over",
    'found'
}

local function main_handler()
    local obj_arr = {}
    local oid_arr = {}
    local result = ""
    local mib_obj = {}
    local found_oid = inputOid
    local status = "in_progress"
    local base_oid = "1.3.6.1.4.1.42019"
    local res_obj = {
        status = "",
        oid = "",
        mib_obj = {},
        res = ""
    }

    local count = 1
    local error_count = 0
    while status ~= "OK" do
        error_count = error_count + 1
        if error_count > 50 then
            return "while_not_stopped"
        end

        if count == 1 then
            --print("inputOid " .. inputOid)
            oid_arr = snmp.get_oidArr(inputOid)
            count = count + 1
        elseif count > 1 then
            --print("res_obj.oid = " .. res_obj.oid)
            oid_arr = snmp.get_oidArr(res_obj.oid)
        end
        --print("main status = " .. status)
        if status == "in_progress" then
            -----------------------------------------------------
            obj_arr, status = snmp.get_OidElement(oid_arr)
            -------------------------------------------------------
        elseif status == "oid_over_list" then
            --print("1 -- status == oid_over_list")
            local new_arr = {}
            oid_arr = snmp.get_oidArr(inputOid)
            for i = 1, 4 do
                new_arr[i] = oid_arr[i]
            end
            local prev_obj = {}
            local prev_list = {}
            if #oid_arr > 3 then
                prev_obj = obj_arr[3]
                prev_list = prev_obj[3]
                if new_arr[#new_arr] + 1 > #prev_list then
                    status = "oid_over_list"
                else
                    new_arr[#new_arr] = new_arr[#new_arr] + 1
                    status = "in_progress"
                end
            end

            if status == "oid_over_list" then
                if #oid_arr > 2 then
                    prev_obj = obj_arr[2]
                    prev_list = prev_obj[3]
                    new_arr = {}
                    for i = 1, 3 do
                        new_arr[i] = oid_arr[i]
                    end

                    if new_arr[#new_arr] + 1 > #prev_list then
                        status = "failed"
                        break
                    else
                        new_arr[#new_arr] = new_arr[#new_arr] + 1
                        status = "in_progress"
                    end
                end
            end
            if status == "in_progress" then
                oid_arr = {}
                oid_arr = new_arr
                res_obj.oid = base_oid
                for i = 1, #new_arr do
                    res_obj.oid = res_obj.oid .. "." .. new_arr[i]
                end
            end
            found_oid = res_obj.oid
        elseif status == "over_index" then
            local new_arr = {}
            oid_arr = snmp.get_oidArr(inputOid)
            for i = 1, #oid_arr - 2 do
                new_arr[i] = oid_arr[i]
            end
            new_arr[#oid_arr - 1] = oid_arr[#oid_arr - 1] + 1
            oid_arr = {}
            oid_arr = new_arr
            res_obj.oid = base_oid
            for i = 1, #oid_arr do
                res_obj.oid = res_obj.oid .. "." .. oid_arr[i]
            end
            status = "in_progress"
        elseif status == "oid_over" then
            --return 0
        elseif status == "not_defined" then
            --return 0
            break
        elseif status == "zero_index" then
            local last_element = {}
            local prev_element = {}
            local current_list = {}
            local current_oid = ""
            local current_oid_arr = {}
            last_element = obj_arr[#obj_arr]

            if last_element[4] == "scalar" then
                prev_element = obj_arr[#obj_arr - 1]
                current_list = prev_element[3]
                --print("#current_list = " .. #current_list)
                current_oid = last_element[1]
                current_oid_arr = snmp.get_oidArr(current_oid)
                local last_index = current_oid_arr[#current_oid_arr]
                local next_index = tonumber(last_index) + 1

                if next_index <= #current_list then
                    current_oid_arr[#current_oid_arr] = next_index
                else
                    current_oid = prev_element[1]
                    current_oid_arr = snmp.get_oidArr(current_oid)
                    last_index = current_oid_arr[#current_oid_arr]
                    next_index = tonumber(last_index) + 1
                    current_oid_arr[#current_oid_arr] = next_index
                end

                res_obj.oid = base_oid
                for i = 1, #current_oid_arr do
                    res_obj.oid = res_obj.oid .. "." .. current_oid_arr[i]
                end
            end
            status = "in_progress"
        elseif status == "failed" then
            --return 0
            break
        elseif status == "found" then

            if requestType == "GET" then
                res_obj = snmp.GET_requestHandler(obj_arr)
                status = res_obj.status
            elseif requestType == "GET_NEXT" then
                if #obj_arr > 1 then
                    local input_oid_arr = snmp.get_oidArr(found_oid)
                    res_obj = snmp.GET_NEXT_requestHandler(obj_arr, input_oid_arr)
                    status = res_obj.status
                elseif #obj_arr == 1 then
                    res_obj.oid = base_oid .. "." .. "3.1"
                    status = "in_progress"
                end
            elseif requestType == "SET" then
                res_obj.mib_obj = {}
                res_obj = snmp.SET_requestHandler(obj_arr, setParam)
                status = res_obj.status
            end
        end
    end
    send_Get_result(res_obj.oid, res_obj.mib_obj, res_obj.res)
    return status
end

local function tf_run_snmp()

    local file = io.open("/tmp/testsetRequest", "w")
    local err = 0
    if arg[1] ~= nil then
        err = 1
        file:write("arg 1 = " .. arg[1] .. "\n")
    end
    if arg[2] ~= nil then
        err = 2
        file:write("arg 2 = " .. arg[2] .. "\n")
    end
    if arg[3] ~= nil then
        err = 3
        file:write("arg 3 = " .. arg[3] .. "\n")
    end
    if arg[4] ~= nil then
        err = 4
        file:write("arg 4 = " .. arg[4] .. "\n")
    end

    file:write("error " .. err .. "\n")
    io.close(file)

    requestType = snmp.getRequestType(requestType)
    --print("requestType = " .. requestType)
    if requestType == nil or requestType == "" then
        print("requestType = nil")
        return 0
    elseif requestType == "SET" then
        if arg[3] == nil or arg[3] == "" then
            return 0
        else
            setParam.valueType = arg[3]
        end

        if arg[4] == nil or arg[4] == "" then
            return 0
        else
            setParam.value = arg[4]
        end
    end
    local res = {}
    local in_endIndex = string.len(inputOid)
    local count = 0
    if inputOid == ".1.3.6.1.4.1.42019" then
        count = snmp.getFirstOidIndex()
        inputOid = inputOid .. "." .. count
    end

    --file = io.open("/tmp/logfile_input", "w")
    --file:write("oid = " .. inputOid .. "\n")
    --file.close()

    if arg[2] ~= nil then
        main_handler()
    else
        print("OID must be presented arg[2]")
    end
end

tf_run_snmp()
