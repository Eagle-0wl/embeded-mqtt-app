include $(TOPDIR)/rules.mk

PKG_NAME:=app_mqtt
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/app_mqtt
	CATEGORY:=Base system
	TITLE:=app_mqtt
	#DEPENDS:=+libibm +libubus +libubox +libblobmsg-json
endef

define Package/app_mqtt/description
	This is an example app package which uses an example library package
endef

define Package/app_mqtt/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	#$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app_mqtt $(1)/usr/bin
	#$(INSTALL_BIN) ./files/mqtt.init $(1)/etc/init.d/mqtt
	$(INSTALL_CONF) ./files/mosquitto $(1)/etc/config/mosquitto
endef

$(eval $(call BuildPackage,app_mqtt))
