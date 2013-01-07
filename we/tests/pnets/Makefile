
MAKEFILE += Makefile.sequence

.PHONY: comparison

comparison:
	$(foreach FILE,$(MAKEFILE), $(MAKE) -f $(FILE) comparison)
