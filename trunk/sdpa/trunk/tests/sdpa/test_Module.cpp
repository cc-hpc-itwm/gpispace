#include <iostream>

#include "test_Module.hpp"
#include <sdpa/modules/modules.hpp>
#include <sdpa/modules/Module.hpp>
#include <sdpa/modules/ModuleLoader.hpp>
#include <sdpa/util.hpp>

using namespace sdpa;
using namespace sdpa::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( sdpa::tests::ModuleTest );

void ModuleTest::setUp() {
}

void ModuleTest::tearDown() {
}

void ModuleTest::testModuleLoad() {
}

void ModuleTest::testModuleUnLoad() {
  sdpa::modules::ModuleLoader::ptr_t loader = sdpa::modules::ModuleLoader::create();
  try {
    loader->load("example-mod", "./libexample-mod.so");
  } catch (sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }
} 

void ModuleTest::testModuleFunctionCall() {
  sdpa::modules::ModuleLoader::ptr_t loader = sdpa::modules::ModuleLoader::create();
  try {
    loader->load("example-mod", "./libexample-mod.so");
  } catch (sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }
  sdpa::modules::Module &mod = loader->get("example-mod");
  sdpa::modules::Module::data_t params;
  params["out"] = sdpa::wf::Parameter("out", sdpa::wf::Parameter::OUTPUT_EDGE, sdpa::wf::Token());
  mod.call("HelloWorld", params);
  CPPUNIT_ASSERT_EQUAL(std::string("hello world"), params["out"].token().data());
}

void ModuleTest::testModuleIllegalFunctionCall() {
  sdpa::modules::ModuleLoader::ptr_t loader = sdpa::modules::ModuleLoader::create();
  try {
    loader->load("example-mod", "./libexample-mod.so");
  } catch (const sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }
  sdpa::modules::Module &mod = loader->get("example-mod");
  sdpa::modules::Module::data_t data;

  try {
    mod.call("NonExistingFunction", data);
  } catch (const sdpa::modules::FunctionNotFound &fnf) {
    // ok
  } catch (const std::exception &ex) {
    CPPUNIT_ASSERT_MESSAGE(std::string("function call failed: ") + ex.what(), false);
  } catch (...) {
    CPPUNIT_ASSERT_MESSAGE(std::string("function call failed: unknown reason"), false);
  }
}

void ModuleTest::testModuleLoopingCall() {
  sdpa::modules::ModuleLoader::ptr_t loader = sdpa::modules::ModuleLoader::create();
  try {
    loader->load("example-mod", "./libexample-mod.so");
  } catch (const sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }
  sdpa::modules::Module &mod = loader->get("example-mod");
  sdpa::modules::Module::data_t data;

  sdpa::util::time_type start = sdpa::util::now();
  for (std::size_t i = 0; i < 100000; ++i) {
    mod.call("DoNothing", data);
  }
  sdpa::util::time_type end = sdpa::util::now();

  std::cout << "call loop took " << (end - start) << "usec" << std::endl;
}
