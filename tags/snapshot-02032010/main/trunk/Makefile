CMAKE=cmake
CMAKE_CONFIG=
CTEST_CONFIG=
BUILD_DIR=build
top_srcdir=${CURDIR}
GWES_CPP_HOME=$(shell pwd)/gwes
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

clean-sdpa:
	$(MAKE) -C $(BUILD_DIR)/sdpa clean

distclean-sdpa: clean-sdpa
	@-rm -rf $(BUILD_DIR)/sdpa

clean:
	$(MAKE) -C $(BUILD_DIR) clean

doc-api: config
	$(MAKE) -C ${BUILD_DIR} doc

doc:
	$(MAKE) -C doc

dist: test
	$(MAKE) -C $(BUILD_DIR) package

##test: 
##	make -C $(BUILD_DIR) -e GWES_CPP_HOME=${GWES_CPP_HOME} test

test:	ctest
ctest: config
	cd $(BUILD_DIR) && GWES_CPP_HOME=${GWES_CPP_HOME} ctest -D ExperimentalStart -D ExperimentalConfigure -D ExperimentalBuild -D ExperimentalTest -D ExperimentalCoverage ${CTEST_CONFIG}

unify:
	(CTEST_TAG=$(shell head -1 ${BUILD_DIR}/Testing/TAG); \
	mv -i ${BUILD_DIR}/Testing/$${CTEST_TAG} ${BUILD_DIR}/Testing/Result; \
	mv -i ${LOGDIR}/LastBuild_$${CTEST_TAG}.log ${LOGDIR}/LastBuild.log; \
	mv -i ${LOGDIR}/LastConfigure_$${CTEST_TAG}.log ${LOGDIR}/LastConfigure.log; \
	mv -i ${LOGDIR}/LastCoverage_$${CTEST_TAG}.log ${LOGDIR}/LastCoverage.log; \
	mv -i ${LOGDIR}/LastTest_$${CTEST_TAG}.log ${LOGDIR}/LastTest.log )
##	mv -i ${LOGDIR}/LastTestsFailed_$${CTEST_TAG}.log ${LOGDIR}/LastTestsFailed.log

.DEFAULT:
	$(MAKE) -C $(BUILD_DIR) $(MAKECMDGOALS)

.PHONY: clean distclean all default doc test clean-sdpa distclean-sdpa
