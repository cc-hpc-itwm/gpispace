#include "EncoderTest.hpp"

#include <string>

#include <seda/comm/Encoder.hpp>

class Foo {
  public:
    Foo() : valid(false) {}

    std::string encode() const {
      return "hello world!";
    }

    void decode(const std::string &data) {
      if (data == "hello world!") {
        valid = true;
      } else {
        valid = false;
      }
    }

    bool valid;
};

using namespace seda::comm::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( EncoderTest );

EncoderTest::EncoderTest()
  : SEDA_INIT_LOGGER("tests.seda.comm.EncoderTest")
{}

void
EncoderTest::setUp() {
}

void
EncoderTest::tearDown() {
}

void
EncoderTest::testEncode() {
  Foo foo;
  std::string data = seda::comm::Encoder::encode(foo);
  foo.decode(data);
  CPPUNIT_ASSERT(foo.valid);
}
