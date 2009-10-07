CMAKE=cmake
BUILD_DIR=build
top_srcdir=${CURDIR}

default: compile
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

compile: config
	make -C $(BUILD_DIR)

config: $(BUILD_DIR) CMakeLists.txt
	cd $(BUILD_DIR) && $(CMAKE) $(top_srcdir)

distclean:
	@-rm -rf $(BUILD_DIR)
	@-find . -name "*~" -delete

doc: config
	make -C build doc

.DEFAULT:
	@cd $(BUILD_DIR) && $(MAKE) -$(MAKEFLAGS) $(MAKECMDGOALS)

.PHONY: clean distclean all default doc
