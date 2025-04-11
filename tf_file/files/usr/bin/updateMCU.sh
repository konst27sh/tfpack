#!/bin/bash

verNumber=0
i2cStatus=""
dir="/usr/share/mcu_firmware"
currentVersion=0
files=$(ls $dir)
filename="mcu_"
filenameType=".bin"
pathMCU=""
if [[ "$files" =~ \.bin ]]; then
    regex="mcu_([0-9]+)\.bin"
    for file in $files; do
        if [[ $file =~ $regex ]]; then
            verNumber="${BASH_REMATCH[1]}"
        fi
    done
fi

i2cStatus=$(/etc/init.d/tf_hwsysd status)

if [[ "${i2cStatus}" == "running" ]]; then

	currentVersion=$(ubus call tf_hwsys getParam '{"name":"SW_VERS"}' | grep -o '"SW_VERS": "[^"]*"' | awk -F'"' '{print $4}')

	pathMCU+="${dir}"
	pathMCU+="/"
	pathMCU+="${filename}"
	pathMCU+="${verNumber}"
	pathMCU+="${filenameType}"

	echo ${pathMCU}


	if [[ "${currentVersion}" -eq "${verNumber}" ]]; then
	  echo "installed Actual MCU version"
	else
	  ubus call tf_hwsys config '{"name":"close_port","value":"1"}'
	  sleep 1
	  ubus call tf_hwsys setParam '{"name":"BOOT0","value":"1"}'
	  sleep 1
	  ubus call tf_hwsys setParam '{"name":"IOMCU_RESET","value":"2"}'
	  sleep 1
	  stm32flash -w "${pathMCU}" -a 0x3b /dev/i2c-0
	  sleep 1
	  ubus call tf_hwsys setParam '{"name":"BOOT0","value":"0"}'
	  sleep 1
	  ubus call tf_hwsys setParam '{"name":"IOMCU_RESET","value":"2"}'
	  sleep 1
	  ubus call tf_hwsys config '{"name":"open_port","value":"1"}'
	  sleep 10
	  currentVersion=$(ubus call tf_hwsys getParam '{"name":"SW_VERS"}' | grep -o '"SW_VERS": "[^"]*"' | awk -F'"' '{print $4}')
	fi

	if [[ "${i2cStatus}" != "running" ]]; then
	  logger -t FlashMCU -p ERROR "I2C can not run"
	fi

	if [[ "${currentVersion}" -ne "${verNumber}" ]]; then
	  logger -t tfortis -p ERROR "IOMCU firmware updated failed, Version not corrected"
	  logger -t tfortis -p ERROR "IOMCU version installed:   " "${currentVersion}"
	  logger -t tfortis -p ERROR "IOMCU version required:    " "${verNumber}"
	else
	  logger -t tfortis -p INFO "IOMCU firmware updated successfully: ${currentVersion} -> ${verNumber}"
	fi
fi
