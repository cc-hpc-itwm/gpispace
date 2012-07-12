print("Number of places: " .. #pnet:places())

for p in pnet:places():all() do
	print(" - Place " .. p:name())
end

print("Number of transitions: " .. #pnet:transitions())
for t in pnet:transitions():all() do
	print(" - Transition " .. t:name())
	for port in t:ports():all() do
		print("   - Port " .. port:name())
	end
end
