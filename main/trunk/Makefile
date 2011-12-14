#
# obsolete Makefile, please try to use cmake directly
#
CMAKE=cmake
CMAKE_CONFIG=
CTEST_CONFIG=
BUILD_DIR=build
top_srcdir=${CURDIR}
CTEST_TAG2=`head -1 ${BUILD_DIR}/Testing/TAG`
LOGDIR=${BUILD_DIR}/Testing/Temporary

default: compile
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

compile: config
	$(MAKE) -C $(BUILD_DIR)

config: $(BUILD_DIR) CMakeLists.txt
	@echo "make BUILD_DIR=${BUILD_DIR} CMAKE_CONFIG=${CMAKE_CONFIG}"
	(cd $(BUILD_DIR) && $(CMAKE) ${CMAKE_CONFIG} $(top_srcdir))

distclean: clean
	@-rm -rf $(BUILD_DIR)
	@-find . -name "*~" -delete

clean:
	$(MAKE) -C $(BUILD_DIR) clean

dist: test
	$(MAKE) -C $(BUILD_DIR) package

test:	ctest
ctest: config
	cd $(BUILD_DIR) && ctest Experimental ${CTEST_CONFIG}

.DEFAULT:
	$(MAKE) -C $(BUILD_DIR) $(MAKECMDGOALS)

.PHONY: clean distclean all default test dist
