require "pnet.common"
require "pnet.opt.simple_pipe_elimination"

function optimize(pnet)
	simple_pipe_elimination(pnet)
end

apply_recursively(pnet, optimize)
