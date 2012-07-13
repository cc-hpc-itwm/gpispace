print("Number of places: " .. #pnet:places())

for p in pnet:places():all() do
	print("Place `" .. p:name() .. "' with id " .. p:id())
	for port in p:in_connections() do
		print("incoming connection from port `" .. tostring(port) .. "' of the transition `" .. tostring(port:transition()) .. "'")
	end
	for port in p:out_connections() do
		print("outgoing connection to port `" .. tostring(port) .. "' of the transition `" .. tostring(port:transition()) .. "'")
	end
end

print("Number of transitions: " .. #pnet:transitions())
for t in pnet:transitions():all() do
	print("Transition `" .. t:name() .. "'")
	for port in t:ports():all() do
		print("Port `" .. port:name() .. "' connected to place " .. tostring(port:place())
		      .. (port:isInput() and ", is input" or "")
		      .. (port:isOutput() and ", is output" or "")
		      .. (port:isTunnel() and ", is tunnel" or ""))
	end
end
