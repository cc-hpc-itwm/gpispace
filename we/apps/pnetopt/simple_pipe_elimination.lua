local M = {}

local pnet = require("pnet")
local list = require("list")

local function find_matching_places(transition) 
	local input_ports = pnet.name_port_map(pnet.input_ports(transition:ports()))
	local output_ports = pnet.name_port_map(pnet.output_ports(transition:ports()))

	if #input_ports ~= #output_ports then
		return nil
	end

	local result = {}

	for name,input_port in pairs(input_ports) do
		if input_port:isRead() then
			return nil
		end

		local output_port = output_ports[name]

		if output_port == nil or
		   input_port:connectedPlace() == nil or
		   output_port:connectedPlace() == nil then
			return nil
		end

		if list.count(pnet.input_ports(output_port:connectedPlace():connectedPorts())) > 1 or
		   list.count(pnet.output_ports(input_port:connectedPlace():connectedPorts())) > 1 then
			return nil
		end

		-- Anything else?

		result[input_port:connectedPlace()] = output_port:connectedPlace()
	end

	return result
end

local function merge_places(net, first, second)
	for port in list.clone(first:connectedPorts()) do
		port:connect(second)
	end

	local first_has_prefix = first:name():byte(1) == '_'
	local second_has_prefix = second:name():byte(1) == '_'

	if second_has_prefix and not first_has_prefix or
	   first_has_prefix == second_has_prefix and first:name():len() < second:name():len()
	then
		second:setName(first:name())
	end

	first:remove()

	return second
end

function M.simple_pipe_elimination(net)
	local modified = false

	for transition in list.clone(net:transitions()) do
		if transition:valid() and
		   transition:expression() ~= nil and
		   transition:expression():isEmpty() and
		   transition:condition():isConstTrue() then
			local matching_places = find_matching_places(transition)

			if matching_places ~= nil then
				for first,second in pairs(matching_places) do
					merge_places(net, first, second)
				end
				transition:remove()
				modified = true
			end
		end
	end

	return modified
end

return M
