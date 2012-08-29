local M = {}

local list = require("list")

function M.dump(pnet)
	print("Number of places: " .. #pnet:places())

	for p in pnet:places():all() do
		print("Place `" .. p:name() .. "' with id " .. p:id())
		for port in p:inConnections() do
			print("incoming connection from port `" .. tostring(port) .. "' of the transition `" .. tostring(port:transition()) .. "'")
			assert(port:place())
			assert(port:place() == p)
		end
		for port in p:outConnections() do
			print("outgoing connection to port `" .. tostring(port) .. "' of the transition `" .. tostring(port:transition()) .. "'")
			assert(port:place())
			assert(port:place() == p)
		end
	end

	print("Number of transitions: " .. #pnet:transitions())
	for t in pnet:transitions():all() do
		print("Transition `" .. t:name() .. "'")
		for port in t:ports():all() do
			print("Port `" .. port:name() .. "'"
			      .. (port:place() and (", connected to place `" .. tostring(port:place()) .. "'") or "")
			      .. (port:associatedPlace() and (", associated with `" .. tostring(port:associatedPlace()) .. "'") or "")
			      .. (port:isInput() and ", is input" or "")
			      .. (port:isOutput() and ", is output" or "")
			      .. (port:isTunnel() and ", is tunnel" or "")
			)
		end
	end
end

function M.apply_recursively(pnet, functor)
	functor(pnet)

	for t in pnet:transitions():all() do
		if t:subnet() then
			M.apply_recursively(t:subnet(), functor)
		end
	end
end

function M.remove_all_places(pnet)
	for place in list.clone(pnet:places():all()) do
		place:remove()
	end
end

function M.remove_all_transitions(pnet)
	for transition in list.clone(pnet:transitions():all()) do
		transition:remove()
	end
end

function M.input_ports(transition)
	return list.filter(transition.ports().all(), function(port) return port:isInput() end)
end

function M.output_ports(transition)
	return list.filter(transition.ports().all(), function(port) return port:isOutput() end)
end

return M
