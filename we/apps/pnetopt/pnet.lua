local M = {}

local list = require("list")

--- Dumps contents of a Petri net to stdout.
--
-- @param pnet		Petri net.
--
function M.dump(pnet)
	print("Number of places: " .. #pnet:places())

	for p in pnet:places() do
		print("Place " .. p:name())
		for port in p:connectedPorts() do
			print("connected to the port " .. port:name() .. " of the transition " .. port:transition():name())
			assert(p == port:connectedPlace())
		end
		for port in p:associatedPorts() do
			print("associated with the port " .. port:name() .. " of the transition " .. port:transition():name())
			assert(p == port:associatedPlace())
		end
	end

	print("Number of transitions: " .. #pnet:transitions())
	for t in pnet:transitions() do
		print("Transition `" .. t:name() .. "'")
		for port in t:ports() do
			print("Port " .. port:name()
			      .. (port:connectedPlace() and (", connected to place `" .. port:connectedPlace():name() .. "'") or "")
			      .. (port:associatedPlace() and (", associated with place `" .. port:associatedPlace():name() .. "'") or "")
			      .. (port:isInput() and ", is input" or "")
			      .. (port:isOutput() and ", is output" or "")
			      .. (port:isTunnel() and ", is tunnel" or "")
			)
		end
	end
end

--- Applies a functor to a Petri net and to all its subnets.
--
-- @param pnet		Petri net.
-- @param functor	Functor to apply.
--
function M.apply_recursively(pnet, functor)
	functor(pnet)

	for t in pnet:transitions() do
		if t:subnet() then
			M.apply_recursively(t:subnet(), functor)
		end
	end
end

--- Removes all places in a Petri net.
--
-- @param pnet		Petri net.
--
function M.remove_all_places(pnet)
	for place in list.clone(pnet:places()) do
		place:remove()
	end
end

--- Removes all transitions in a Petri net.
--
-- @param pnet		Petri net.
--
function M.remove_all_transitions(pnet)
	for transition in list.clone(pnet:transitions()) do
		transition:remove()
	end
end

--- Takes only input ports from a given list.
--
-- @param ports		Port list iterator.
--
-- @return An iterator to the filtered list.
--
function M.input_ports(ports)
	return list.filter(ports, function(port) return port:isInput() end)
end

--- Takes only output ports from a given list.
--
-- @param ports		Port list iterator.
--
-- @return An iterator to the filtered list.
--
function M.output_ports(ports)
	return list.filter(ports, function(port) return port:isOutput() end)
end

--- Translates a port list to a table, where a key is a port name, and a value is the port with the name.
--
-- @param ports		Port list iterator.
--
-- @return Table mapping port names to ports.
--
function M.name_port_map(ports)
	local result = {}
	for port in ports do
		result[port:name()] = port
	end
	return result
end

return M
