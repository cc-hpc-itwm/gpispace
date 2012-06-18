print(pnet)
print(pnet:transitions())
print(#pnet:transitions())
print(pnet:transitions():all())

it = pnet:transitions():all()

for t in pnet:transitions():all() do
	print(t:name())
end
