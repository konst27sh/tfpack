#!/bin/bash

echo "Enter user password:"
read -s  password

/etc/tf_clish/scripts/tools/transfer_file.lua "${@}" "${password}"