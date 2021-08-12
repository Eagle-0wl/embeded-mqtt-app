--module("luci.controller.mqtt_client", package.seeall)

--function index()
	--entry({"admin", "services", "mqtt_model"}, cbi("mqtt_client"), _("MQTT Client"),105)
--end


module("luci.controller.mqtt_client", package.seeall)

function index()

  entry( { "admin", "services", "mqtt_client"}, firstchild(), _("MQTT Client"), 150)
  entry( { "admin", "services", "mqtt_client", "subscriber" }, cbi("mqtt_model"), _("Subscriber"), 1).leaf = true
  --entry( { "admin", "services", "mqtt_client", "publisher" }, cbi("mqtt_pub"), _("Publisher"), 2).leaf = true
end
