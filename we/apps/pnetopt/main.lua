local pnet = require("pnet")
local spe  = require("simple_pipe_elimination")
local list = require("list")

local net = transition:subnet()
assert(net, "top-level transition must have a subnet")

if false then
	pnet.apply_recursively(net, pnet.dump)

	print("Removing places...")
	pnet.apply_recursively(net, pnet.remove_all_places)

	print("Printing everything...")
	pnet.apply_recursively(net, pnet.dump)

	print("Removing transitions...")
	pnet.apply_recursively(net, pnet.remove_all_transitions)

	print("Printing everything...")
	pnet.apply_recursively(net, pnet.dump)
else
	pnet.apply_recursively(net, spe.simple_pipe_elimination)
end
