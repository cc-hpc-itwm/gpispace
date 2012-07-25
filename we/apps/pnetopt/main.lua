require "pnet.common"
require "pnet.opt.simple_pipe_elimination"

function remove_all_transitions_naive(pnet)
	for transition in pnet:transitions():all() do
		transition:remove()
	end
end

function remove_all_transitions_clever(pnet)
	local transitions = {}
	for transition in pnet:transitions():all() do
		table.insert(transitions, transition)
	end

	for i,transition in ipairs(transitions) do
		transition:remove()
	end
end

function optimize(pnet)
	simple_pipe_elimination(pnet)
end

-- apply_recursively(pnet, optimize)
apply_recursively(pnet, dump)
apply_recursively(pnet, remove_all_transitions_clever)
