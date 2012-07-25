local pnet = require("pnet")
local spe  = require("simple_pipe_elimination")

pnet.apply_recursively(net, spe.simple_pipe_elimination)
-- pnet.apply_recursively(net, pnet.remove_all_transitions)
-- pnet.apply_recursively(net, pnet.remove_all_places)
-- pnet.apply_recursively(net, pnet.dump)
