print("----------------")
print(" - Number of transitions: " .. #pnet:transitions())

for t in pnet:transitions():all() do
	print(t:name())
end

print(" - Number of places: " .. #pnet:places())
for p in pnet:places():all() do
	print(p:name())
end
