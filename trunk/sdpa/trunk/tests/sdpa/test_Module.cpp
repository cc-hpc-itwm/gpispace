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
  sdpa::modules::ModuleLoader::Ptr loader = sdpa::modules::createModuleLoader();
  try {
    loader->load("example-mod", "./libexample-mod.so");
  } catch (sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }
} 

void ModuleTest::testModuleFunctionCall() {
  sdpa::modules::ModuleLoader::Ptr loader = sdpa::modules::createModuleLoader();
  try {
    loader->load("example-mod", "./libexample-mod.so");
  } catch (sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }
  sdpa::modules::Module &mod = loader->get("example-mod");
  sdpa::modules::Module::input_data_t in;
  sdpa::modules::Module::output_data_t out;

  in.push_back(sdpa::Token(42));
  mod.call("HelloWorld", in, out);
  CPPUNIT_ASSERT_EQUAL((sdpa::modules::Module::output_data_t::size_type)1, out.size());
  sdpa::Token token(out.front());
  CPPUNIT_ASSERT_EQUAL(std::string("hello world"), token.as<std::string>());
}

void ModuleTest::testModuleIllegalFunctionCall() {
  sdpa::modules::ModuleLoader::Ptr loader = sdpa::modules::createModuleLoader();
  try {
    loader->load("example-mod", "./libexample-mod.so");
  } catch (const sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }
  sdpa::modules::Module &mod = loader->get("example-mod");
  sdpa::modules::Module::input_data_t in;
  sdpa::modules::Module::output_data_t out;

  try {
    mod.call("NonExistingFunction", in, out);
  } catch (const sdpa::modules::FunctionNotFound &fnf) {
    // ok
  } catch (const std::exception &ex) {
    CPPUNIT_ASSERT_MESSAGE(std::string("function call failed: ") + ex.what(), false);
  } catch (...) {
    CPPUNIT_ASSERT_MESSAGE(std::string("function call failed: unknown reason"), false);
  }
}

void ModuleTest::testModuleLoopingCall() {
  sdpa::modules::ModuleLoader::Ptr loader = sdpa::modules::createModuleLoader();
  try {
    loader->load("example-mod", "./libexample-mod.so");
  } catch (const sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }
  sdpa::modules::Module &mod = loader->get("example-mod");
  sdpa::modules::Module::input_data_t in;

  long long start = sdpa::util::now();
  for (std::size_t i = 0; i < 100000; ++i) {
    sdpa::modules::Module::output_data_t out;
    mod.call("DoNothing", in, out);
  }
  long long end = sdpa::util::now();

  std::cout << "call loop took " << (end - start) << "usec" << std::endl;
}
