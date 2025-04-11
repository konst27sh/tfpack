---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by sheverdin.
--- DateTime: 3/12/24 10:55 AM
---

local tf_utilities = {}
local tf = require "tf_module"

function tf_utilities.separateLettersAndNumbers(inputString)
    local letters = ""
    local numbers = ""
    for i = 1, #inputString do
        local char = inputString:sub(i, i)
        if tonumber(char) ~= nil then
            numbers = numbers .. char
        else
            letters = letters .. char
        end
    end
    return letters, tonumber(numbers)
end

function tf_utilities.is_valid_time(time)
    local pattern = "(%d+):(%d+)"
    local hh, mm = string.match(time, pattern)

    if mm == nil or hh == nil then
        return false
    end
    mm = tonumber(mm)
    hh = tonumber(hh)
    if mm < 0 or mm > 59 or hh < 0 or hh > 24 then
        print("error: time is not valid")
        return false
    end
    return true
end

function tf_utilities.is_valid_ip(ip)
    local parts = { ip:match("(%d+)%.(%d+)%.(%d+)%.(%d+)") }
    if #parts ~= 4 then
        print("IP not valid")
        return false
    end
    for _, part in ipairs(parts) do
        if tonumber(part) < 0 or tonumber(part) > 255 then
            print("IP not valid")
            return false
        end

        if string.len(part) > 1 and string.sub(part, 1, 1) == '0' then
            print("IP not valid")
            return false
        end
    end
    return true
end

function tf_utilities.icmp_ping(host)
    local command = string.format("ping -c 1 %s >> /dev/null", host)
    local status = os.execute(command)
    if status == 0 then
        --print(string.format("%s connected.", host))
        return true
    else
        --print(string.format("%s not connected.", host))
        return false
    end
end

function tf_utilities.convertSeconds(seconds)
    local date = {}
    date.days = math.floor(seconds / 86400)
    seconds = seconds % 86400
    date.hours = math.floor(seconds / 3600)
    seconds = seconds % 3600
    date.minutes = math.floor(seconds / 60)
    date.sec = seconds % 60
    return date
end

return tf_utilities
