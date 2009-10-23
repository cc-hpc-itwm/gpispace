#include <iostream>

#include <fhglog/fhglog.hpp>

#include "test_ActivityExecutor.hpp"
#include <sdpa/modules/ActivityExecutor.hpp>
#include <sdpa/util/util.hpp>

using namespace sdpa;
using namespace sdpa::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( sdpa::tests::ActivityExecutorTest );

void ActivityExecutorTest::setUp()
{

}

void ActivityExecutorTest::tearDown()
{

}

void ActivityExecutorTest::testModuleNotLoaded()
{
  sdpa::modules::ModuleLoader::ptr_t loader = sdpa::modules::ModuleLoader::create();
  sdpa::modules::ActivityExecutor executor(loader);

  sdpa::wf::Activity a1("activity-1"
                      , sdpa::wf::Method("example-mod@HelloWorld"));
  try
  {
    executor.execute(a1);
    CPPUNIT_ASSERT_MESSAGE("execution should throw since the module has not been loaded", false);
  }
  catch (const std::exception &ex)
  {
    // ok
  }
}


void ActivityExecutorTest::testOneActivity()
{
  sdpa::modules::ModuleLoader::ptr_t loader = sdpa::modules::ModuleLoader::create();
  loader->load("example-mod", "./libexample-mod.so");
  sdpa::modules::ActivityExecutor executor(loader);

  sdpa::wf::Activity a1("activity-1", sdpa::wf::Method("example-mod@HelloWorld"));

  a1.add_parameter(sdpa::wf::Parameter("in", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token(42)));
  a1.add_parameter(sdpa::wf::Parameter("out", sdpa::wf::Parameter::OUTPUT_EDGE, sdpa::wf::Token("")));
  try
  {
    MLOG(INFO, "going to execute: " << a1);
    executor.execute(a1);
    CPPUNIT_ASSERT_EQUAL(std::string("hello world"), a1.parameters()["out"].token().data());
    MLOG(INFO, "sucessfully executed activity: " << a1);
  }
  catch (const std::exception &ex)
  {
    MLOG(ERROR, "exception during activity execution: " << ex.what());
    CPPUNIT_ASSERT_MESSAGE(std::string("exception during activity execution: ")+ex.what(), false);
  }
}

