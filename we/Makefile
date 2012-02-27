CMAKE=cmake
CMAKE_CONFIG=
BUILD_DIR=build
top_srcdir=${CURDIR}

default: compile
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

compile: $(BUILD_DIR)/Makefile depend
	$(MAKE) -C $(BUILD_DIR)

config: $(BUILD_DIR) CMakeLists.txt
	cd $(BUILD_DIR) && $(CMAKE) $(CMAKE_CONFIG) $(top_srcdir)

distclean: clean
	@-rm -rf $(BUILD_DIR)
	@-find . -name "*~" -delete

clean:
	$(MAKE) -C $(BUILD_DIR) clean

doc-api: config
	$(MAKE) -C $(BUILD_DIR) doc

doc:
	$(MAKE) -C doc

tags:
	@ctags -e -R --c++-kinds=+p --fields=+iaS --extra=+q --languages=C++ --language-force=C++ -f TAGS .

ctest: test
test: config
	cd $(BUILD_DIR) && ctest -D ExperimentalStart -D ExperimentalConfigure -D ExperimentalBuild -D ExperimentalTest -R $(TESTCASES)
cover: config
	cd $(BUILD_DIR) && ctest -D ExperimentalStart -D ExperimentalConfigure -D ExperimentalBuild -D ExperimentalTest -D ExperimentalCoverage
memchk: config
	cd $(BUILD_DIR) && ctest -D ExperimentalStart -D ExperimentalConfigure -D ExperimentalBuild -D ExperimentalMemCheck

depend:
	$(MAKE) -C $(BUILD_DIR) depend

.DEFAULT:
	@$(MAKE) -C $(BUILD_DIR) -$(MAKEFLAGS) $(MAKECMDGOALS)

.PHONY: clean distclean all default doc tags ctest test cover memchk
