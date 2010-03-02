#include <iostream>

#include "test_Module.hpp"
#include <sdpa/modules/modules.hpp>
#include <sdpa/modules/Module.hpp>
#include <sdpa/modules/ModuleLoader.hpp>
#include <sdpa/util/util.hpp>

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
    loader->load("./libexample-mod.so");
  } catch (sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }
} 

void ModuleTest::testModuleFunctionCall() {
  sdpa::modules::ModuleLoader::ptr_t loader = sdpa::modules::ModuleLoader::create();
  try {
    loader->load("./libexample-mod.so");
  } catch (sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }
  sdpa::modules::Module &mod = loader->get("example-mod");
  sdpa::modules::data_t params;
  params["out"] = sdpa::wf::Parameter("out", sdpa::wf::Parameter::OUTPUT_EDGE, sdpa::wf::Token());
  mod.call("HelloWorld", params);
  CPPUNIT_ASSERT_EQUAL(std::string("hello world"), params["out"].token().data());
}

void ModuleTest::testModuleIllegalFunctionCall() {
  sdpa::modules::ModuleLoader::ptr_t loader = sdpa::modules::ModuleLoader::create();
  try {
    loader->load("./libexample-mod.so");
  } catch (const sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }
  sdpa::modules::Module &mod = loader->get("example-mod");
  sdpa::modules::data_t data;

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
    loader->load("./libexample-mod.so");
  } catch (const sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }
  sdpa::modules::Module &mod = loader->get("example-mod");
  sdpa::modules::data_t data;

  sdpa::util::time_type start = sdpa::util::now();
  for (std::size_t i = 0; i < 100000; ++i) {
    mod.call("DoNothing", data);
  }
  sdpa::util::time_type end = sdpa::util::now();

  std::cout << "call loop took " << (end - start) << "usec" << std::endl;
}

void ModuleTest::testAddFunctionCall() {
  sdpa::modules::ModuleLoader::ptr_t loader = sdpa::modules::ModuleLoader::create();
  try {
    loader->load("./libexample-mod.so");
  } catch (const sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }

  sdpa::modules::Module &mod = loader->get("example-mod");
  sdpa::modules::data_t params;
  long long sum(0);
  const long long max(100000);
  const long long expected( (max*max) / 2 - (max / 2));

  std::cout << "summing from 0 to " << max << " ";
  std::cout.flush();
  params["out"] = sdpa::wf::Parameter("out", sdpa::wf::Parameter::OUTPUT_EDGE, sdpa::wf::Token((long long)0));

  sdpa::util::time_type start = sdpa::util::now();
  for (long long i(0); i < max; ++i)
  {
    params["a"] = sdpa::wf::Parameter("a", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token((long long)i));
    params["b"] = sdpa::wf::Parameter("a", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token((long long)sum));
    try {
      mod.call("Add", params);
    } catch (const std::exception & ex) {
      CPPUNIT_ASSERT_MESSAGE(std::string("function call failed: ") + ex.what(), false);
    }
    sum = params["out"].token().data_as<long long>();
    if ((i % 10000) == 0) { std::cout << "."; std::cout.flush(); }
  }
  sdpa::util::time_type end = sdpa::util::now();

  std::cout << sum << std::endl;
  std::cout << "loop took " << (end - start) << "usec" << std::endl;

  CPPUNIT_ASSERT_EQUAL(expected, sum);
  CPPUNIT_ASSERT_EQUAL(sum, params["out"].token().data_as<long long>());
}

void ModuleTest::testAlloc() {
  sdpa::modules::ModuleLoader::ptr_t loader = sdpa::modules::ModuleLoader::create();
  try {
    loader->load("./libexample-mod.so");
  } catch (const sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }

  sdpa::modules::Module &mod = loader->get("example-mod");
  sdpa::modules::data_t params;

  std::clog << "allocating " << sizeof(int) << " bytes" << std::endl;
  params["size"] = sdpa::wf::Parameter("size", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token(sizeof(int)));
  mod.call("Malloc", params);
  {
    int *i = (int*)params["out"].token().data_as<void*>();
    *i = 42;
  }
  std::clog << "deallocating " << params["out"].token().data_as<void*>() << std::endl;
  params["ptr"] = sdpa::wf::Parameter("ptr", sdpa::wf::Parameter::INPUT_EDGE, params["out"].token());
  mod.call("Free", params);
  std::clog << "ptr-value after free: " << params["ptr"].token().data_as<void*>() << std::endl;
  try {
    mod.call("Free", params);
    CPPUNIT_ASSERT_MESSAGE("calling Free twice should result in a double-free detection", false);
  } catch (const std::exception &ex) {
    // OK - free detected a double-free ;-)
  }
}

void ModuleTest::testUpdate() {
  sdpa::modules::ModuleLoader::ptr_t loader = sdpa::modules::ModuleLoader::create();
  try {
    loader->load("./libexample-mod.so");
  } catch (const sdpa::modules::ModuleLoadFailed &mlf) {
    CPPUNIT_ASSERT_MESSAGE(std::string("module loading failed:") + mlf.what(), false);
  }

  sdpa::modules::Module &mod = loader->get("example-mod");
  sdpa::modules::data_t params;

  std::clog << "allocating " << sizeof(int) << " bytes" << std::endl;
  params["size"] = sdpa::wf::Parameter("size", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token(sizeof(int)));
  mod.call("Malloc", params);

  params["ptr"] = sdpa::wf::Parameter("ptr", sdpa::wf::Parameter::UPDATE_EDGE, params["out"].token());
  params["value"] = sdpa::wf::Parameter("ptr", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token(0xdeadbeef));

  mod.call("Update", params);

  // check the value
  {
    unsigned int *ptr = (unsigned int*)params["ptr"].token().data_as<void*>();
    CPPUNIT_ASSERT_EQUAL(0xdeadbeef, *ptr);
  }

  std::clog << "deallocating " << params["ptr"].token().data_as<void*>() << std::endl;
  params["ptr"] = sdpa::wf::Parameter("ptr", sdpa::wf::Parameter::INPUT_EDGE, params["out"].token());
  mod.call("Free", params);
  std::clog << "ptr-value after free: " << params["ptr"].token().data_as<void*>() << std::endl;
  try {
    mod.call("Free", params);
    CPPUNIT_ASSERT_MESSAGE("calling Free twice should result in a double-free detection", false);
  } catch (const std::exception &ex) {
    // OK - free detected a double-free ;-)
  }
}
