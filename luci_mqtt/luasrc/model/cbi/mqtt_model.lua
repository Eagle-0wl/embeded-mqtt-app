
require("luci.config")
local fw = require "luci.model.firewall"
local deathTrap = { }
local m, s, s2
local certs = require "luci.model.certificate"
local certificates = certs.get_certificates()
local keys = certs.get_keys()
local cas = certs.get_ca_files().certs

m = Map("mosquitto")
m:chain("firewall")
fw.init(m.uci)

---------------------------- Config ----------------------------

s2 = m:section(NamedSection, "mqtt", "mqtt", translate("Config"), "")
s2:tab("misc",  translate("Subscriber"))
s2:tab("security", translate("SSL"))
s2:tab("client", translate("Topic"))

s2.template = "mqtt_client/tsection"
s2.anonymous = true
FileUpload.unsafeupload = true

function s2.cfgsections(self)
	return {"mqtt"}
end
---------------------------- Security Tab ----------------------------

use_tls_ssl = s2:taboption("security", Flag, "use_tls_ssl", translate("Use TLS/SSL"), translate("Mark to use TLS/SSL for connection"))
use_tls_ssl.rmempty = false
function use_tls_ssl.write(self, section, value)
	self.map:set("mqtt", self.option, value)
end

tls_type = s2:taboption("security", ListValue, "tls_type", translate("TLS Type"), translate("Select the type of TLS encryption"))
tls_type:depends("use_tls_ssl", "1")
tls_type:value("cert", translate("Certificate based"))
tls_type:value("psk", translate("Pre-Shared-Key based"))

local certificates_link = luci.dispatcher.build_url("admin", "system", "admin", "certificates")
o = s2:taboption("security", Flag, "_device_sec_files", translate("Certificate files from device"),
	translatef("Choose this option if you want to select certificate files from device.\
	Certificate files can be generated <a class=link href=%s>%s</a>", certificates_link, translate("here")))
o:depends({use_tls_ssl="1", tls_type = "cert"})

ca_file = s2:taboption("security", FileUpload, "ca_file", translate("CA File"), translate("Upload CA file"));
ca_file:depends({use_tls_ssl="1",_device_sec_files="", tls_type = "cert"})

cert_file = s2:taboption("security", FileUpload, "cert_file", translate("CERT File"), translate("Upload CERT file"));
cert_file:depends({use_tls_ssl="1",_device_sec_files="", tls_type = "cert"})

key_file = s2:taboption("security", FileUpload, "key_file", translate("Key File"), translate("Upload Key file"));
key_file:depends({use_tls_ssl="1",_device_sec_files="", tls_type = "cert"})

ca_file = s2:taboption("security", ListValue, "_device_ca_file", translate("CA File"), translate("Upload CA file"));
ca_file:depends({_device_sec_files = "1", tls_type = "cert"})

if #cas > 0 then
	for _,ca in pairs(cas) do
		ca_file:value("/etc/certificates/" .. ca.name, ca.name)
	end
else 
	ca_file:value("", translate("-- No file available --"))
end

function ca_file.write(self, section, value)
	m.uci:set(self.config, section, "ca_file", value)
end

ca_file.cfgvalue = function(self, section)
	return m.uci:get(m.config, section, "ca_file") or ""
end

cert_file = s2:taboption("security", ListValue, "_device_cert_file", translate("CERT File"), translate("Upload CERT file"));
cert_file:depends({_device_sec_files = "1", tls_type = "cert"})

if #certificates > 0 then
	for _,cert in pairs(certificates) do
		cert_file:value("/etc/certificates/" .. cert.name, cert.name)
	end
else 
	cert_file:value("", translate("-- No file available --"))
end

function cert_file.write(self, section, value)
	m.uci:set(self.config, section, "cert_file", value)
end

cert_file.cfgvalue = function(self, section)
	return m.uci:get(m.config, section, "cert_file") or ""
end

key_file = s2:taboption("security", ListValue, "_device_key_file", translate("Key File"), translate("Upload Key file"));
key_file:depends({_device_sec_files = "1", tls_type = "cert"})

if #keys > 0 then
	for _,key in pairs(keys) do
		key_file:value("/etc/certificates/" .. key.name, key.name)
	end
else 
	key_file:value("", translate("-- No file available --"))
end

function key_file.write(self, section, value)
	m.uci:set(self.config, section, "key_file", value)
end

key_file.cfgvalue = function(self, section)
	return m.uci:get(m.config, section, "key_file") or ""
end

tls_version = s2:taboption("security", ListValue, "tls_version", translate("TLS version"), translate("Used TLS version"));
tls_version:depends({tls_type = "cert"})
tls_version:value("tlsv1", "tlsv1");
tls_version:value("tlsv1.1", "tlsv1.1");
tls_version:value("tlsv1.2", "tlsv1.2");
tls_version:value("all", "Support all");
tls_version.default = "all"

o = s2:taboption("security", Value, "psk", translate("Pre-Shared-Key"), translate("The pre-shared-key in hex format with no leading “0x”"))
o.datatype = "lengthvalidation(0, 128)"
o.placeholder = "Key"
o:depends({use_tls_ssl = "1", tls_type = "psk"})

o = s2:taboption("security", Value, "identity", translate("Identity"), translate("Specify the Identity"))
o.datatype = "uciname"
o.placeholder = "Identity"
o:depends({use_tls_ssl = "1", tls_type = "psk"})




client_enabled = s2:taboption("client", Flag, "client_enabled", translate("Enable"), translate("Enable connection to remote bridge"))
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

---------------------------- Misc Tab ----------------------------

--enb = s2:taboption(Flag, "enabled", translate("Enable"), translate("Select to enable MQTT"))
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
password.datatype = "string"
password.maxlength = "256"

return m
