// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnetv_verify
#include <boost/test/unit_test.hpp>

#include <pnetv/jpna/Parsing.h>
#include <pnetv/jpna/PetriNet.h>
#include <pnetv/jpna/Verification.h>

#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random_string.hpp>

FHG_BOOST_TEST_LOG_VALUE_PRINTER (jpna::VerificationResult::Result, os, result)
{
  os <<
    ( (result == jpna::VerificationResult::TERMINATES) ? "TERMINATES"
    : (result == jpna::VerificationResult::LOOPS) ? "LOOPS"
    : (result == jpna::VerificationResult::MAYBE_LOOPS) ? "MAYBE_LOOPS"
    : throw std::runtime_error ("STRANGE verification_result")
    );
}

BOOST_AUTO_TEST_CASE (isolated_transition_terminates)
{
  we::type::transition_t transition
    ( fhg::util::testing::random_string()
    , we::type::expression_t()
    , boost::none
    , true
    , we::type::property::type()
    , we::priority_type()
    );
  we::type::activity_t activity (transition, boost::none);
  boost::ptr_vector<jpna::PetriNet> nets;
  jpna::parse (fhg::util::testing::random_string().c_str(), activity, nets);
  BOOST_REQUIRE_EQUAL (nets.size(), 1);
  BOOST_REQUIRE_EQUAL ( jpna::verify (nets.front()).result()
                      , jpna::VerificationResult::TERMINATES
                      );
}
