#! /usr/bin/env lua

---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by sheverdin.
--- DateTime: 4/1/25 11:29 AM
---

local md5   = require "md5"
local tf    = require "tf_module"

local uci = require("luci.model.uci").cursor()
local cursor = uci.cursor()

local function get_hash()
    local cmd = "fw_printenv ethaddr 2>&1"
    local temp = tostring(tf.executeCommand(cmd))
    if (string.find(temp, "ethaddr")) then
        temp = string.gsub(temp, "ethaddr=", "")
        temp = string.gsub(temp, ":", "")
        temp = string.sub(temp, 1, -2)
        --print("md5 str = " .. temp .. "+" .. "admin" .. "+" .. "admin")
        local err = md5.sumhexa(temp .. "+" .. "admin" .. "+" .. "admin")
        return err
    end
end

local function change_hash()
    local file = io.open("/tmp/debug_hash.txt", "w")
    if arg[1] == nil then
        file:write("arg 1 is null \n")
        print(1)
        return
    end
    local hash_res = 1001
    local user = arg[1]
    file:write("befor while \n")
    local admin_hash = get_hash()
    file:write("user = " .. user .. "  admin_hash = " .. admin_hash .. "\n")
    local hash_cmd = "uci -q set rpcd." .. user .. ".hash=" .. admin_hash
    file:write("hash_cmd " .. hash_cmd .. "\n")
    hash_res = os.execute(hash_cmd)
    file:write("hash_res = " .. hash_res .. "\n")
    file:close()
    print(hash_res)
end

change_hash()


