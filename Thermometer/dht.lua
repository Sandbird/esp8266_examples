PIN = 4; -- GPIO 2
DHT = require("dht_lib")

repeat
	DHT.read22(PIN) -- read22 if DHT22 or DHT11 and read11 if DHT11
	t = DHT.getTemperature()
	h = DHT.getHumidity()
	if (h == nil) then
		print("Error reading from DHT22")
		tmr.wdclr()
	elseif (h > 1000) then
		print("Error reading from DHT22")
		tmr.wdclr()
	else
		tempin=(t/10)
		hum=(h/10)
		print(tempin)
		print(hum)
		print("Get numbers from DHT22")
		tmr.wdclr()
	end
until (h ~= nil) and (h < 1000)

DHT = nil
package.loaded["dht_lib"] = nil