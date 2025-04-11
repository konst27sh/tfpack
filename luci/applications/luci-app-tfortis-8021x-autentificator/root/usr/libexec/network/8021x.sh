#!/bin/sh

case ${2:-NOTANEVENT} in
    block_all)
        ip link set dev $1 nomaster
        logger -t "hostapd" -p user.notice "$1: All the bridge traffic blocked. Traffic will be allowed after 802.1x authentication"
        ;;

    AP-STA-CONNECTED | CTRL-EVENT-EAP-SUCCESS | CTRL-EVENT-EAP-SUCCESS2)
        #nft add element bridge tr-mgmt-br-lan1 allowed_macs { $3 }
		ip link set dev $1 master switch
        logger -t "hostapd" -p user.notice "$1: Allowed traffic from $3"
        ;;

    AP-STA-DISCONNECTED | CTRL-EVENT-EAP-FAILURE)
        #nft delete element bridge tr-mgmt-br-lan allowed_macs { $3 }
		ip link set dev $1 nomaster
        logger -t "hostapd" -p user.notice "$1: Denied traffic from $3"
        ;;

    allow_all)
        #nft destroy table bridge "$TABLE"
		#nft table bridge "$TABLE"
		#nft delete table bridge "$TABLE"
		ip link set dev $1 master switch
        logger -t "hostapd" -p user.notice "$1: Allowed all bridge traffice again"
        ;;

    NOTANEVENT)
        logger -t "hostapd" -p user.notice "$0 was called incorrectly, usage: $0 interface event [mac_address]"
        ;;
esac