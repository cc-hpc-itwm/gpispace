local M = {}

local list = require("list")

function M.find_matching_places(transition) 
	if #transition:in_ports() ~= #transition:out_ports() then
		return nil
	end

	local result = {}

	for port_name, in_port in pairs(transition:in_ports()) do
		local out_port = transition:out_ports()[port_name]

		if out_port == nil or
		   in_port:place() == nil or
		   out_port:place() == nil then
			return nil
		end
		if #out_port:place():in_edges() > 1 and
		   #in_port:place():out_edges() > 1 then
			return nil
		end
		if in_port:place():capacity() ~= out_port:place():capacity() then
			return nil
		end
		if in_port:is_read() then
			return nil
		end
		-- Anything else?
		result[in_port:place()] = out_port:place()
	end

	return result
end

function M.simple_pipe_elimination(net)
	local modified = false

	for transition in list.enumerate(net:transitions():all()) do
		if transition:expression() ~= nil and
		   transition:expression():isEmpty() and
		   transition:condition():isConstTrue() then
			local matching_places = find_matching_places(transition)

			if matching_places ~= nil then
				for place1, place2 in pairs(matching_places) do
					-- NB: should reassociate ports when necessary
					merge_places(net, place1, place2)
				end
				transition:remove()
				modified = true
			end
		end
	end

	return modified
end

return M
