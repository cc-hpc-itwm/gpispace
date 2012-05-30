function find_predecessor(transition)
	-- no chance when input and output ports have the same name
	for name in transition:in_ports() do
		if transition:out_ports()[name] ~= nil then
			return nil
		end
	end

	-- places_out: places that are successors of the transition
	local places_out = {}
	for port_name, port in transition:out_ports() do
		places_out[port:place()] = 1
	end
	
	-- places_read: places that are read by the transition
	local places_read = {}
	-- predecessors_read: transitions that output tokens to places_read
	local predecessors_read = {}
	-- predecessors: transitions that output tokens to input places other than places_read
	local predecessors = {}

	local max_successors_of_pred = 0
	for port_name, port in transition:in_ports() do
		if port:is_read() then
			if #port:place():in_edges() = 0 then
				places_read[port:place()] = 1
			else
				for i,edge in ipairs(port:place():in_edges()) do
					predecessors_read[edge:transition()] = port:place()

					for n,p in edge:transition():out_ports() do
						if places_out[p:place()] ~= nil then
							return nil
						end
					end
				end
			end
		elseif #port:place():in_edges() = 0 then
			-- WORK HERE ...
			return nil
		else
			for i,edge in ipairs(port:place():in_edges()) do
				-- Predecessor must be an expression.
				if edge:transition():expression() then
					return nil
				end

				for n,p in edge:transition():out_ports() do
					if places_out[p:place()] ~= nil then
						return nil
					end
				end
				max_successors_of_pred = max(max_successors_of_pred, #edge:transition():out_ports())
				predecessors[edge:transition()] = 1
			end
		end
	end

	if (#predecessors ~= 1 or max_successors_of_pred > 1) then
		return nil
	end

--        What's this?
--
--        const pair_type p (*preds.begin());
--
--        for ( typename set_of_tid_pid_type::const_iterator tr (preds_read.begin())
--            ; tr != preds_read.end()
--            ; ++tr
--            )
--          {
--            if (tr->first != p.second)
--              {
--                pid_read.insert (tr->second);
--              }
--          }
--
--        return trans_info (p.first, p.second, pid_read);
	

	for transition,v in pairs(predecessors_read) do
		return transition, places_read
	end

	return nil
end

function merge_expressions(net)
	local modified = false

	for transition_name, transition in pairs(net:transitions()) do
		if transition:expression() ~= nil and
		   transition:condition():is_const_true() then
		   	predecessor = find_predecessor(transition)

			-- To be continued
		end
	end

	return modified;
end
