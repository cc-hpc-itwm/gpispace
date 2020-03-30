#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/optional/optional_io.hpp>

#include <we/type/activity.hpp>
#include <we/type/transition.hpp>
#include <we/type/net.hpp>
#include <we/type/signature.hpp>
#include <we/type/value.hpp>
#include <we/type/value/wrap.hpp>

#include <util-generic/testing/random.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace
{
  namespace signature = pnet::type::signature;
  namespace value = pnet::type::value;

  we::type::eureka_id_type const eureka_id ("group");

  size_t const MAX_TOKENS = 1000;

  struct net_with_eureka_transition
  {
    std::string const eureka_condition;
    we::type::net_type net;
    we::place_id_type pid_in;

    net_with_eureka_transition ( we::type::eureka_id_type const& id
                               , std::string const& condition
                               );
    void put_tokens (std::vector<long> const&);
  };

  net_with_eureka_transition::net_with_eureka_transition
      ( we::type::eureka_id_type const& eureka_id
      , std::string const& condition
      )
    : eureka_condition (condition)
    , pid_in
      (net.add_place (place::type ( "in"
                                  , signature::signature_type ("long")
                                  , true
                                  )
                     )
      )
  {
    we::type::transition_t trans_eureka
      ( "generate_ids_on_eureka_port"
        , we::type::expression_t ("${out} := set_insert (Set{}, ${id})")
        , we::type::expression_t (eureka_condition)
        , {}
        , we::priority_type()
      );
    we::port_id_type const port_id_eureka_gid
      ( trans_eureka.add_port ( we::type::port_t
                                 ( "id"
                                 , we::type::PORT_IN
                                 , signature::signature_type ("string")
                                 , we::type::property::type()
                                 )
                               )
      );
    we::port_id_type const port_id_in
      ( trans_eureka.add_port ( we::type::port_t
                                 ( "in"
                                 , we::type::PORT_IN
                                 , signature::signature_type ("long")
                                 , we::type::property::type()
                                 )
                               )
      );
    we::port_id_type const port_id_out
      ( trans_eureka.add_port ( we::type::port_t
                                 ( "out"
                                 , we::type::PORT_OUT
                                 , signature::signature_type ("set")
                                 , we::type::property::type()
                                 )
                               )
      );

    we::place_id_type const pid_eureka_gid
      (net.add_place (place::type ( "id"
                                  , signature::signature_type ("string")
                                  , true
                                  )
                     )
      );

    we::type::property::type empty;
    we::transition_id_type const tid (net.add_transition (trans_eureka));

    net.add_connection ( we::edge::PT_READ
                       , tid
                       , pid_eureka_gid
                       , port_id_eureka_gid
                       , empty
                       );
    net.add_connection (we::edge::PT, tid, pid_in, port_id_in, empty);
    net.add_eureka (tid, port_id_out);

    net.put_value (pid_eureka_gid, eureka_id);
  }

  void net_with_eureka_transition::put_tokens
    (std::vector<long> const& values)
  {
    for (auto const& v : values)
    {
      net.put_value (pid_in, v);
    }
  }
}

BOOST_AUTO_TEST_CASE (check_transition_generates_no_eureka_responses)
{
  std::vector<long> tokens;

  long const eureka_value
    ([&tokens]
     {
       //!note generate at least two tokens
       auto const n_tokens ( fhg::util::testing::random<size_t>{}()
                           % MAX_TOKENS
                           + 2
                           );
       tokens = fhg::util::testing::randoms
                < std::vector<long>
                , fhg::util::testing::unique_random
                > (n_tokens);

        long _value = tokens.back();
        tokens.pop_back ();

       return _value;
     }()
    );

  net_with_eureka_transition eureka_net
    ( eureka_id
    , "${in} :eq: " + std::to_string (eureka_value) + "L"
    );
  eureka_net.put_tokens (tokens);

  boost::optional<we::type::eureka_id_type> eureka_received;
  std::mt19937 random_engine
    (fhg::util::testing::detail::GLOBAL_random_engine()());

  BOOST_REQUIRE
    ( !eureka_net.net.fire_expressions_and_extract_activity_random
        ( random_engine
        , [] ( pnet::type::value::value_type const&
             , pnet::type::value::value_type const&
             )
          {
            throw std::logic_error ("got workflow_response unsupported");
          }
        , [&eureka_received] (we::type::eureka_ids_type const& ids)
          {
            BOOST_TEST (ids.size() == 1);
            eureka_received = *ids.begin();
          }
        )
    );

  BOOST_REQUIRE_EQUAL ( eureka_received
                      , boost::none
                      );
}

BOOST_AUTO_TEST_CASE (check_transition_generates_eureka_id_on_eureka_response)
{
  std::vector<long> tokens;

  long const eureka_value
    ([&tokens]
     {
       auto const n_tokens ( fhg::util::testing::random<size_t>{}()
                           % MAX_TOKENS
                           + 1
                           );
       tokens = fhg::util::testing::randoms
                < std::vector<long>
                , fhg::util::testing::unique_random
                > (n_tokens);
       return tokens.at ( fhg::util::testing::random<long>{}()
                        % n_tokens
                        );
     }()
    );

  net_with_eureka_transition eureka_net
    ( eureka_id
    , "${in} :eq: " + std::to_string (eureka_value) + "L"
    );
  eureka_net.put_tokens (tokens);

  boost::optional<we::type::eureka_id_type> eureka_received;
  std::mt19937 random_engine
    (fhg::util::testing::detail::GLOBAL_random_engine()());

  BOOST_REQUIRE
    ( !eureka_net.net.fire_expressions_and_extract_activity_random
        ( random_engine
        , [] ( pnet::type::value::value_type const&
             , pnet::type::value::value_type const&
             )
          {
            throw std::logic_error ("got workflow_response unsupported");
          }
        , [&eureka_received] (we::type::eureka_ids_type const& ids)
          {
            BOOST_TEST (ids.size() == 1);
            eureka_received = *ids.begin();
          }
        )
    );

  BOOST_REQUIRE_EQUAL ( eureka_received
                      , boost::optional<we::type::eureka_id_type> (eureka_id)
                      );
}

BOOST_AUTO_TEST_CASE (check_transition_generates_one_or_more_eureka_responses)
{
  size_t num_eureka_expected = 0;
  std::vector<long> tokens;

  long const eureka_value
    ([&tokens, &num_eureka_expected]
     {
       auto const n_tokens ( fhg::util::testing::random<size_t>{}()
                           % MAX_TOKENS
                           + 1
                           );
       tokens = fhg::util::testing::randoms
                < std::vector<long>
                , fhg::util::testing::unique_random
                > (n_tokens);
       auto const eureka
        (tokens.at (fhg::util::testing::random<long>{}() % n_tokens));
       std::for_each ( tokens.begin()
                     , tokens.end()
                     , [&eureka, &num_eureka_expected] (long const& val)
                       {
                         if (val >= eureka) num_eureka_expected++;
                       }
                     );

       return eureka;
     }()
    );

  net_with_eureka_transition eureka_net
    ( eureka_id
    , "${in} :ge: " + std::to_string (eureka_value) + "L"
    );
  eureka_net.put_tokens (tokens);

  size_t num_eureka_received = 0;
  std::mt19937 random_engine
    (fhg::util::testing::detail::GLOBAL_random_engine()());

  BOOST_REQUIRE
    ( !eureka_net.net.fire_expressions_and_extract_activity_random
        ( random_engine
        , [] ( pnet::type::value::value_type const&
             , pnet::type::value::value_type const&
             )
          {
            throw std::logic_error ("got workflow_response unsupported");
          }
        , [&num_eureka_received] (we::type::eureka_ids_type const& ids)
          {
            BOOST_TEST (ids.size() == 1);
            num_eureka_received++;
          }
        )
    );

  BOOST_REQUIRE_EQUAL (num_eureka_expected, num_eureka_received);
}
