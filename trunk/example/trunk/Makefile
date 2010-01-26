CMAKE=cmake
CMAKE_CONFIG=
BUILD_DIR=build
top_srcdir=${CURDIR}
CTEST_TAG2=`head -1 ${BUILD_DIR}/Testing/TAG`
LOGDIR=${BUILD_DIR}/Testing/Temporary

default: compile
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

compile: $(BUILD_DIR) $(BUILD_DIR)/Makefile depend
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
	@ctags -R --c++-kinds=+p --fields=+iaS --extra=+q --languages=C++ --language-force=C++ -f .tags .

depend:
	$(MAKE) -C $(BUILD_DIR) depend

ctest: test
test: config
	cd $(BUILD_DIR) && ctest -D ExperimentalStart -D ExperimentalConfigure -D ExperimentalBuild -D ExperimentalTest
cover: config
	cd $(BUILD_DIR) && ctest -D ExperimentalStart -D ExperimentalConfigure -D ExperimentalBuild -D ExperimentalTest -D ExperimentalCoverage
memchk: config
	cd $(BUILD_DIR) && ctest -D ExperimentalStart -D ExperimentalConfigure -D ExperimentalBuild -D ExperimentalMemCheck

unify:
	(CTEST_TAG=$(shell head -1 ${BUILD_DIR}/Testing/TAG); \
	mv -i ${BUILD_DIR}/Testing/$${CTEST_TAG} ${BUILD_DIR}/Testing/Result; \
	mv -i ${LOGDIR}/LastBuild_$${CTEST_TAG}.log ${LOGDIR}/LastBuild.log; \
	mv -i ${LOGDIR}/LastConfigure_$${CTEST_TAG}.log ${LOGDIR}/LastConfigure.log; \
	mv -i ${LOGDIR}/LastCoverage_$${CTEST_TAG}.log ${LOGDIR}/LastCoverage.log; \
	mv -i ${LOGDIR}/LastTest_$${CTEST_TAG}.log ${LOGDIR}/LastTest.log )
##	mv -i ${LOGDIR}/LastTestsFailed_$${CTEST_TAG}.log ${LOGDIR}/LastTestsFailed.log

.DEFAULT:
	@$(MAKE) -C $(BUILD_DIR) -$(MAKEFLAGS) $(MAKECMDGOALS)

.PHONY: clean distclean all default doc tags ctest test cover memchk depend
