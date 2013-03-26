local M = {}

local pnet = require("pnet")
local list = require("list")

local function find_matching_places(transition)
	-- Mappings from port names to ports.
	--
	local input_ports = pnet.name_port_map(pnet.input_ports(transition:ports()))
	local output_ports = pnet.name_port_map(pnet.output_ports(transition:ports()))

	if list.table_size(input_ports) ~= list.table_size(output_ports) then
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

		result[input_port:connectedPlace()] = output_port:connectedPlace()
	end

	-- If the input place gives tokens not only to me, then
	--
	-- (1) the output place can't take tokens from anybody except me;
	-- (2) the output place can't be associated to an output port.
	--
	for input_place,output_place in pairs(result) do
		if list.count(pnet.input_ports(input_place:connectedPorts())) + list.count(pnet.output_ports(input_place:associatedPorts())) > 1 and
		   (list.count(pnet.output_ports(output_place:connectedPorts())) + list.count(pnet.input_ports(output_place:associatedPorts())) > 1 or
		    list.count(pnet.output_ports(output_place:associatedPorts())) > 0) then
			return nil
		end
	end

	-- A mistiming problem is possible if we propagate more than one token at a time.
	--
	if (list.table_size(result) > 1) then
		-- There is at most one transition that takes token from my output places
		-- or
		-- there is at most one transition that puts tokens into my input places.
		--
		-- Associations with ports count too.
		--
		local consumers = {}
		local producers = {}
		for input_place,output_place in pairs(result) do
			for port in pnet.output_ports(input_place:connectedPorts()) do
				producers[port:transition()] = 1
			end
			for port in pnet.input_ports(input_place:associatedPorts()) do
				producers["associated ports"] = 1
			end

			for port in pnet.input_ports(output_place:connectedPorts()) do
				consumers[port:transition()] = 1
			end
			for port in pnet.output_ports(output_place:associatedPorts()) do
				consumers["associated ports"] = 1
			end
		end
		if list.table_size(consumers) > 1 and list.table_size(producers) > 1 then
			return nil
		end
	end

	return result
end

local function merge_places(first, second)
	--
	-- Choose the place to keep and the place to remove.
	--
	local first_has_prefix = first:name():byte(1) == '_'
	local second_has_prefix = second:name():byte(1) == '_'

	if second_has_prefix and not first_has_prefix or
	   first_has_prefix == second_has_prefix and first:name():len() < second:name():len()
	then
		first,second = second,first
	end

	--
	-- Reconnect and reassociate ports.
	--
	for port in list.clone(first:connectedPorts()) do
		port:connect(second)
	end
	for port in list.clone(first:associatedPorts()) do
		port:associate(second)
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
					merge_places(first, second)
				end
				transition:remove()
				modified = true
			end
		end
	end

	return modified
end

return M
