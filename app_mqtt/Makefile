include $(TOPDIR)/rules.mk

PKG_NAME:=app_mqtt
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/app_mqtt
	CATEGORY:=Base system
	TITLE:=app_mqtt
	DEPENDS:=+libmosquitto
endef

define Package/app_mqtt/description
	This is an example app package which uses an example library package
endef

define Package/app_mqtt/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/etc/app_mqtt_crt/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app_mqtt $(1)/usr/bin
	$(INSTALL_BIN) ./files/appmqtt.init $(1)/etc/init.d/appmqtt
	$(INSTALL_BIN) ./files/mosquitto.org.crt $(1)/etc/app_mqtt_crt/mosquitto.org.crt
	$(INSTALL_CONF) ./files/mosquitto_client.config $(1)/etc/config/mosquitto_client
endef

$(eval $(call BuildPackage,app_mqtt))
