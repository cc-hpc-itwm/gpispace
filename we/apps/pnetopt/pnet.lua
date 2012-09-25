local M = {}

local list = require("list")

function M.dump(pnet)
	print("Number of places: " .. #pnet:places())

	for p in pnet:places() do
		print("Place `" .. p:name())
		for port in p:connectedPorts() do
			print("connected to the port " .. port:name() .. " of the transition " .. port:transition():name())
			assert(port:connectedPlace())
		end
	end

	print("Number of transitions: " .. #pnet:transitions())
	for t in pnet:transitions() do
		print("Transition `" .. t:name() .. "'")
		for port in t:ports() do
			print("Port " .. port:name()
			      .. (port:connectedPlace() and (", connected to place `" .. port:connectedPlace():name() .. "'") or "")
			      .. (port:isInput() and ", is input" or "")
			      .. (port:isOutput() and ", is output" or "")
			      .. (port:isTunnel() and ", is tunnel" or "")
			)
		end
	end
end

function M.apply_recursively(pnet, functor)
	functor(pnet)

	for t in pnet:transitions() do
		if t:subnet() then
			M.apply_recursively(t:subnet(), functor)
		end
	end
end

function M.remove_all_places(pnet)
	for place in list.clone(pnet:places()) do
		place:remove()
	end
end

function M.remove_all_transitions(pnet)
	for transition in list.clone(pnet:transitions()) do
		transition:remove()
	end
end

function M.input_ports(transition)
	return list.filter(transition.ports(), function(port) return port:isInput() end)
end

function M.output_ports(transition)
	return list.filter(transition.ports(), function(port) return port:isOutput() end)
end

return M
