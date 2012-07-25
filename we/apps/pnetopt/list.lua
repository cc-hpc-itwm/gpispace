local M = {}

function M.list(iterator)
	local result = {}
	for item in iterator do
		table.insert(result, item)
	end
	return result
end

function M.items(list)
	local co = coroutine.create(function()
		for index,item in ipairs(list) do
			coroutine.yield(item)
		end
	end)

	return function()
		local code,res = coroutine.resume(co)
		return res
	end
end

function M.enumerate(iterator)
	return M.items(M.list(iterator))
end

return M
