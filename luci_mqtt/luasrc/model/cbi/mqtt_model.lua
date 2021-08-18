require("luci.config")

local m, s, s2

m = Map("mosquitto_client")

---------------------------- Config ----------------------------

s2 = m:section(NamedSection, "mqtt_client", "mqtt_client", translate("Config"), "")
s2:tab("misc",  translate("Subscriber"))

s2:tab("client", translate("Topic"))

s2.template = "mqtt_client/tsection"
s2.anonymous = true
FileUpload.unsafeupload = true

function s2.cfgsections(self)
	return {"mqtt_client"}
end

use_tls_ssl = s2:taboption("client", Flag, "use_tls_ssl", translate("Use TLS/SSL"), translate("Mark to use TLS/SSL for connection"))
use_tls_ssl.rmempty = false

st = m:section(TypedSection, "topic", translate("Topics"), translate("") )
st.addremove = true
st.anonymous = true
st.template = "mqtt_client/tblsection"
st.novaluetext = translate("There are no topics created yet.")

topic = st:option(Value, "topic", translate("Topic name"), translate(""))
topic.datatype = "string"
topic.maxlength = 65536
topic.placeholder = translate("Topic")
topic.rmempty = false
topic.parse = function(self, section, novld, ...)
	local value = self:formvalue(section)
	if value == nil or value == "" then
		self.map:error_msg(translate("Topic name can not be empty"))
		self.map.save = false
	end
	Value.parse(self, section, novld, ...)
end

qos = st:option(ListValue, "qos", translate("QoS level"), translate("The publish/subscribe QoS level used for this topic"))
qos:value("0", "At most once (0)")
qos:value("1", "At least once (1)")
qos:value("2", "Exactly once (2)")
qos.rmempty=false
qos.default="0"

enb = s2:taboption("misc", Flag, "enable", translate("Enable"))
enb.rmempty = false

adress = s2:taboption( "misc", Value, "adress", translate("Broker adress"))
adress.maxlength = "256"
adress.datatype = "string"

local_port = s2:taboption( "misc", Value, "local_port", translate("Local Port"))
local_port.placeholder = "1883"
local_port.datatype = "port"

username = s2:taboption( "misc", Value, "username", translate("Username"))
username.default = "testuotojas"
username.placeholder = "testuotojas"
username.datatype = "string"
username.maxlength = "256"

password = s2:taboption( "misc", Value, "password", translate("Password"))
password.placeholder = "password"
password.password = true
password.datatype = "string"
password.maxlength = "256"

return m
