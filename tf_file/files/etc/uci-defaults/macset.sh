#!/bin/sh /etc/rc.common

logger() {
  /usr/bin/logger -p debug -t "default_param" "$1"
}

LOG_FILE="/tmp/setMAC"

label_mac=$(fw_printenv ethaddr -n)
[ -z "$label_mac" ] && { echo "Error: MAC address not found"; exit 1; }

if  [ "$label_mac" = "C0:11:A6:00:00:00" ]; then
   exit 1
fi

for section in $(uci show  | grep '\.macaddr=' | cut -d'.' -f2 | cut -d'=' -f1 | sort -u); do
    uci -q set network.$section.macaddr="$label_mac"
done

logger "set mac successful"
echo "set mac $label_mac successful" >> $LOG_FILE

FILE_PATH="/usr/share/lua/change_hash.lua"

if [ ! -f "$FILE_PATH" ]; then
    logger "File $FILE_PATH not found"
    echo "File $FILE_PATH not found" >> $LOG_FILE
    exit 1
fi

if [ ! -x "$FILE_PATH" ]; then
    echo "set +x priv" >> $LOG_FILE
    logger "set +x to script"
    chmod 755 "$FILE_PATH"
fi

ADMIN_HASH=$($FILE_PATH "admin")
echo "ADMIN_HASH = $ADMIN_HASH" >> $LOG_FILE

if [ "$ADMIN_HASH" = 0 ]; then
  uci commit
  reload_config
  echo "new admin hash changed successful" >> $LOG_FILE
  logger "new admin hash changed successful"
  echo "$LOG_STR" >> $LOG_FILE
  exit 0
fi

if [ "$ADMIN_HASH" != 0 ]; then
  echo "new admin hash changed failed" >> $LOG_FILE
  logger "new admin hash changed failed"
  exit 1
fi



