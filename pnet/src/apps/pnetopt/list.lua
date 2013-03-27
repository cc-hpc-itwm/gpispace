local M = {}

M.append = table.insert

--- Translates a list iterator to a list.
--
-- @param iterator	A list iterator.
--
-- @return A list (Lua table) consisting of all the elements given by the iterator.
--
function M.list(iterator)
	local result = {}
	for item in iterator do
		M.append(result, item)
	end
	return result
end

--- Creates an iterator for a list represented as a table.
--
-- @param	A list (Lua table).
--
-- @return An iterator for this list.
--
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

--- Creates an independent copy of an iterator.
--
-- @param iterator Iterator.
--
-- @return An iterator over a list created from the input iterator.
--
function M.clone(iterator)
	return M.iterator(M.list(iterator))
end

--- Filters elements matching a predicate.
--
-- @param iterator Iterator.
-- @param predicate Predicate.
--
-- @return An iterator returning only those elements returned by the input iterator which match the predicate.
--
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

--- Counts the number of elements returned by an iterator.
--
-- @param iterator Iterator.
--
-- @return The amount of elements the iterator will return before returning Nil.
--
function M.count(iterator)
	local result = 0
	while iterator() ~= nil do
		result = result + 1
	end
	return result
end

--- Returns the number of key-value pairs in the table.
--
-- (Weird, but #table returns only the size of the "array part" of the table.)
--
-- @param table
--
-- @return The number of key-value pairs in the table.
--
function M.table_size(table)
	local size = 0
	for key,value in pairs(table) do
		size = size + 1
	end
	return size
end

return M
