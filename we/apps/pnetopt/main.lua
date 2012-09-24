print(net)

print("Number of places: " .. #net:places())
for p in net:places() do
	print("Place " .. p:name())
end

print("Number of transitions: " .. #net:transitions())
for t in net:transitions() do
	print("Transition " .. t:name())
	
	for port in t:ports() do
		print("Port `" .. port:name() .. "'"
			.. (port:connectedPlace() and (", connected to place " .. port:connectedPlace():name()) or "")
			.. (port:isInput() and ", is input" or "")
			.. (port:isOutput() and ", is output" or "")
			.. (port:isTunnel() and ", is tunnel" or ""))
	end
end

--local pnet = require("pnet")
--local spe  = require("simple_pipe_elimination")

-- pnet.apply_recursively(net, spe.simple_pipe_elimination)
-- pnet.apply_recursively(net, pnet.remove_all_transitions)
-- pnet.apply_recursively(net, pnet.remove_all_places)
--pnet.apply_recursively(net, pnet.dump)
