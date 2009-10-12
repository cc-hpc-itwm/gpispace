CMAKE=cmake
CMAKE_CONFIG=
BUILD_DIR=build
top_srcdir=${CURDIR}

default: compile
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

compile: config
	make -C $(BUILD_DIR)

config: $(BUILD_DIR) CMakeLists.txt
	cd $(BUILD_DIR) && $(CMAKE) $(top_srcdir)

distclean: clean
	@-rm -rf $(BUILD_DIR)
	@-find . -name "*~" -delete

clean:
	make -C $(BUILD_DIR) clean

doc-api: config
	make -C ${BUILD_DIR} doc

doc:
	make -C doc

dist: test
	make -C $(BUILD_DIR) package

test: ctest
ctest: config
	cd $(BUILD_DIR) && ctest -D ExperimentalStart -D ExperimentalConfigure -D ExperimentalBuild -D ExperimentalTest -D ExperimentalCoverage

.DEFAULT:
	make -C $(BUILD_DIR) $(MAKECMDGOALS)

.PHONY: clean distclean all default doc test
