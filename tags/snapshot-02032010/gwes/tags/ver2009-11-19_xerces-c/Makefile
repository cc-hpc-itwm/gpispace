CMAKE=cmake
CMAKE_CONFIG=
CTEST_CONFIG=
BUILD_DIR=build
top_srcdir=${CURDIR}
GWES_CPP_HOME=`pwd`


default: compile
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

compile: config
	$(MAKE) -C $(BUILD_DIR)

config: $(BUILD_DIR) CMakeLists.txt
	cd $(BUILD_DIR) && $(CMAKE) ${CMAKE_CONFIG} $(top_srcdir)

release: configrelease
	$(MAKE) -C $(BUILD_DIR)

configrelease: $(BUILD_DIR) CMakeLists.txt
	cd $(BUILD_DIR) && $(CMAKE) $(top_srcdir) -DCMAKE_BUILD_TYPE=Release

distclean:
	@-rm -rf $(BUILD_DIR)
	@-find . -name "*~" -delete

test:
	$(MAKE) -C $(BUILD_DIR) test -e GWES_CPP_HOME=${GWES_CPP_HOME}

.DEFAULT:
	$(MAKE) -C $(BUILD_DIR) $(MAKECMDGOALS)

.PHONY: clean distclean all default
