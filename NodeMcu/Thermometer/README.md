# **ESP8266 Thermometer using NodeMCU with DHT22 and DS18B20** #
Tested on version: _nodemcu_float_0.9.6-dev_20150406_

# Info
- [Thingspeak channel](https://thingspeak.com/channels/38407)
- [NodeMCU dev download](https://github.com/nodemcu/nodemcu-firmware/releases/)
- Use nodemcu float version to get float numbers, integer versions to get integer numbers

# Change log
24.09.2015
- Changed old Thingspeak IP to domain name

21.05.2015
- Trying to fix bugs

20.05.2015
- First commit.
- Some code fixes.
- Some readme fixes.
- Trying to fix unstable sending

# Bugs
- Sending are unstable sometimes - _trying to fix_.
- Sometimes DHT gets weird values, probably doubled

# Start playing

#### Edit init.lua
```lua
--Edit 3 line
wifi.sta.config("SSID","password")
```

#### Edit dht.lua
```lua
--Edit 1 line to change GPIO
PIN = 4;
```

#### Edit ds.lua
```lua
--Edit 1 line to change GPIO
pin = 3;
```

#### Edit send.lua
```lua
--Edit 1 line to change your Thingspeak apikey
apiKey = "Thingspeak apikey"
```

#### Compile and clean everything on ESP8266
```lua
--Compile
node.compile("dht.lua")
node.compile("dht_lib.lua")
node.compile("ds.lua")
node.compile("send.lua")
--Clean
file.remove("dht.lua")
file.remove("dht_lib.lua")
file.remove("ds.lua")
file.remove("send.lua")
```
#### Example connections
![Schematic](https://raw.githubusercontent.com/toyorg/nodemcu_thermometer/master/schematic.png)