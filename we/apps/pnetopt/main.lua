print(net)

print(#net:places())
for p in net:places() do
	print(p)
end

print(#net:transitions())
for t in net:transitions() do
	print(t)
end

--local pnet = require("pnet")
--local spe  = require("simple_pipe_elimination")

-- pnet.apply_recursively(net, spe.simple_pipe_elimination)
-- pnet.apply_recursively(net, pnet.remove_all_transitions)
-- pnet.apply_recursively(net, pnet.remove_all_places)
--pnet.apply_recursively(net, pnet.dump)
