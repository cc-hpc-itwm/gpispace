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
	cd $(BUILD_DIR) && $(CMAKE) $(CMAKE_CONFIG) $(top_srcdir)

distclean: clean
	@-rm -rf $(BUILD_DIR)
	@-find . -name "*~" -delete

clean:
	make -C $(BUILD_DIR) clean

doc-api: config
	make -C build doc

doc:
	make -C doc

tags:
	@ctags -R --c++-kinds=+p --fields=+iaS --extra=+q --languages=C++ --language-force=C++ -f .tags .

test: ctest
ctest: config
	cd $(BUILD_DIR) && ctest -D ExperimentalStart -D ExperimentalConfigure -D ExperimentalBuild -D ExperimentalTest -D ExperimentalCoverage

.DEFAULT:
	@cd $(BUILD_DIR) && $(MAKE) -$(MAKEFLAGS) $(MAKECMDGOALS)

.PHONY: clean distclean all default doc tags ctest test
