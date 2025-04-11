#
# This software is licensed under the Public Domain.21
#
include $(TOPDIR)/rules.mk

#date 28.03.2025
PKG_NAME:=tf_autorestart
PKG_VERSION:= 0.1
PKG_RELEASE:= 6

PKG_MAINTAINER:=Konstantin Shevedrin <sheverdin@fort-telecom.ru>
PKG_LICENSE:=CC0-1.0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/tf_autorestart
	SECTION:=tfortis
	# Select package by default
	DEFAULT:=y
	CATEGORY:=TFortis
	DEPENDS:=+jansson
	TITLE:=	App for control and autorestrt PoE
	URL:=https://www.tfortis.com
endef

define Package/tf_autorestart/description
	App for control and autorestrt PoE
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/lib
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./lib/* $(PKG_BUILD_DIR)/lib
endef

define Package/tf_autorestart/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tf_autorestart_ctrl $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tf_autorestartd $(1)/usr/bin/
endef

$(eval $(call BuildPackage,tf_autorestart))
