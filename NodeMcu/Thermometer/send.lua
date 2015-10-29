apiKey = "Thingspeak apikey" -- Add thingspeak api key
conn = nil
success = false
conn = net.createConnection(net.TCP, 0)
conn:on("connection",
   function(conn, payload)
   print("Connected")
   conn:send('GET /update?key='..apiKey..'&field1='..tempin..'&field2='..hum..'&field3='..tempout..'\r\n\
   Host: api.thingspeak.com\r\nAccept: */*\r\nUser-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n\r\n')end)
conn:on("receive", function(conn, payload)success = true print(payload)end)
conn:on("disconnection", function(conn, payload) print('Disconnected') end)
conn:connect(80,'api.thingspeak.com')
if success == true then node.restart() end