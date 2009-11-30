CMAKE=cmake
BUILD_DIR=build
top_srcdir=${CURDIR}

default: compile
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

compile: config
	$(MAKE) -C $(BUILD_DIR)

config: $(BUILD_DIR) CMakeLists.txt
	cd $(BUILD_DIR) && $(CMAKE) $(top_srcdir)

distclean: clean
	@-rm -rf $(BUILD_DIR)
	@-find . -name "*~" -delete

clean:
	$(MAKE) -C $(BUILD_DIR) clean

doc-api: config
	$(MAKE) -C build doc

doc:
	$(MAKE) -C doc

tags:
	@ctags -R --c++-kinds=+p --fields=+iaS --extra=+q --languages=C++ --language-force=C++ -f .tags .

.DEFAULT:
	@cd $(BUILD_DIR) && $(MAKE) -$(MAKEFLAGS) $(MAKECMDGOALS)

.PHONY: clean distclean all default doc tags
