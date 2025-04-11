# See /LICENSE for more information.
# This is free software, licensed under the GNU General Public License v2.

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI TFortis 802.1X Autentificator
LUCI_NAME:=luci-app-tfortis-8021x-autentificator
LUCI_DEPENDS:=+luci-base
PKG_VERSION:=1

#define Package/$(LUCI_NAME)/postinst
define Build/Configure
	#chmod +x $(PKG_BUILD_DIR)/etc/init.d/8021x
	#chmod +x $(PKG_BUILD_DIR)/usr/libexec/network/8021x.sh
endef

include ../../luci.mk
# call BuildPackage - OpenWrt buildroot signature