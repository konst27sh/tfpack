#!/usr/bin/lua

---
--- Generated by Luanalysis
--- Created by sheverdin.
--- DateTime: 2/14/24 5:35 PM
---

local tf = require "tf_module"

local function main_revert(config_type, cmd_list)
    if type(config_type) == "table" then
        for _, section in pairs(config_type) do
            local revert_cmd = cmd_list.cmd_ubus .. cmd_list.cmd_revert .. cmd_list.cmd_prefix .. cmd_list.cmd_config .. section .. cmd_list.cmd_suffix
            tf.executeCommand(revert_cmd)
            io.write("info: configuration \t*", section, "*\t was reverted\n")
        end
    elseif type(config_type) == "string" then
        local revert_cmd = cmd_list.cmd_ubus .. cmd_list.cmd_revert .. cmd_list.cmd_prefix .. cmd_list.cmd_config .. config_type .. cmd_list.cmd_suffix
        tf.executeCommand(revert_cmd)
        io.write("info: configuration \t*", config_type, "*\t was reverted\n")
    end
end

local function showChanges()
    local section_list = tf.changes(tf.ubusList)
    if section_list == nil then
        print("error: changes")
    elseif #section_list == 0 then
        print("\tinfo: no changes")
    end
end

local function revertSection()
    local section = arg[2]
    local flag = 0
    local section_list = tf.changes(tf.ubusList)
    if section == "all" then
        main_revert(section_list, tf.ubusList)
        flag = 1
    else
        for _, sectionName in pairs(section_list) do
            if (sectionName == section) then
                main_revert(section, tf.ubusList)
                flag = 1
                break
            end
        end
    end
    if flag == 0 then
        print("info: no changes for this config type")
    end
end

local function saveChanges()
    tf.saveChanges(1)
end

local change_cmdList = {
    { cmd = "show",      func = showChanges },
    { cmd = "revert",    func = revertSection },
    { cmd = "save",      func = saveChanges },
}

local function run_module()
    for _, value in pairs(change_cmdList) do
        if arg[1] == value.cmd then
            value.func()
        end
    end
end

local function main_changes()
    run_module(change_cmdList)
end

main_changes()
