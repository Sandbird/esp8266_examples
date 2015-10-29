print("Setting up WIFI...")
wifi.setmode(wifi.STATION)
wifi.sta.config("SSID","password") -- Change it
wifi.sta.connect()
tmr.alarm(1, 1000, 1, function()
	if (wifi.sta.getip()== nil) then
		print("IP unavaiable, Waiting...")
	else 
		tmr.stop(1)
		print("Config done, IP is "..wifi.sta.getip())
	end

tmr.alarm(0, 600000, 1, function() -- Do every 10 minutes
	dofile("ds.lc")
	dofile("dht.lc")
	dofile("send.lc")
	tmr.wdclr()
end)
end)