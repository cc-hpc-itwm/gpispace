local M = {}

M.append = table.insert

function M.list(iterator)
	local result = {}
	for item in iterator do
		M.append(result, item)
	end
	return result
end

function M.iterator(list)
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

function M.clone(iterator)
	return M.iterator(M.list(iterator))
end

function M.filter(iterator, predicate)
	local co = coroutine.create(function()
		for item in iterator do
			if predicate(item) then
				coroutine.yield(item)
			end
		end
	end)

	return function()
		local code,res = coroutine.resume(co)
		return res
	end
end

function M.count(iterator)
	local result = 0
	while iterator() ~= Nil do
		result = result + 1
	end
	return result
end

return M
