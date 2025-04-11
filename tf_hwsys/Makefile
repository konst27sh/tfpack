#
# This software is licensed under the Public Domain.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tf_hwsys

PKG_VERSION:= 1.0
PKG_RELEASE:= 4

PKG_MAINTAINER:=Belyaev Alex <abelyaev@fort-telecom.ru>
PKG_LICENSE:=CC0-1.0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/tf_hwsys
	SECTION:=tfortis
	# Select package by default
	DEFAULT:=y
	CATEGORY:=TFortis
	DEPENDS:=+libgpiod +libubox +libubus +libuci
	TITLE:=App for communicate with periphery (Sensors, UPS, SFP), includes Daemon and Controller
	URL:=https://www.tfortis.com
endef

define Package/tf_hwsys/description
	App for communicate with periphery (Sensors, UPS, SFP), includes Daemon and Controller
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/lib
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./lib/* $(PKG_BUILD_DIR)/lib
endef

define Package/tf_hwsys/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tf_hwsysd $(1)/usr/bin/
endef



$(eval $(call BuildPackage,tf_hwsys))
