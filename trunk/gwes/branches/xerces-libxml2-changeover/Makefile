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

release: configrelease
	make -C $(BUILD_DIR)

configrelease: $(BUILD_DIR) CMakeLists.txt
	cd $(BUILD_DIR) && $(CMAKE) $(top_srcdir) -DCMAKE_BUILD_TYPE=Release

distclean:
	@-rm -rf $(BUILD_DIR)
	@-find . -name "*~" -delete

.DEFAULT:
	make -C $(BUILD_DIR) $(MAKECMDGOALS)

.PHONY: clean distclean all default
