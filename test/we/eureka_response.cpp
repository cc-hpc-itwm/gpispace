// Copyright (C) 2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/Transition.hpp>
#include <gspc/we/type/net.hpp>
#include <gspc/we/type/signature.hpp>
#include <gspc/we/type/value.hpp>
#include <gspc/we/type/value/wrap.hpp>

#include <gspc/testing/printer/optional.hpp>
#include <gspc/testing/random.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace
{
  namespace signature = gspc::pnet::type::signature;

  gspc::we::type::eureka_id_type const eureka_id ("group");

  size_t const MAX_TOKENS = 1000;

  struct net_with_eureka_transition
  {
    std::string const eureka_condition;
    gspc::we::type::net_type net;
    gspc::we::place_id_type pid_in;

    net_with_eureka_transition ( gspc::we::type::eureka_id_type const& id
                               , std::string const& condition
                               );
    void put_tokens (std::vector<long> const&);
  };

  net_with_eureka_transition::net_with_eureka_transition
      ( gspc::we::type::eureka_id_type const& eureka_id_
      , std::string const& condition
      )
    : eureka_condition (condition)
    , pid_in
      (net.add_place (gspc::we::type::place::type ( "in"
                                  , signature::signature_type ("long")
                                  , true
                                  , std::nullopt
                                  , gspc::we::type::property::type{}
                                  , gspc::we::type::place::type::Generator::No{}
                                  )
                     )
      )
  {
    gspc::we::type::Transition trans_eureka
      ( "generate_ids_on_eureka_port"
        , gspc::we::type::Expression ("${out} := set_insert (Set{}, ${id})")
        , gspc::we::type::Expression (eureka_condition)
        , {}
        , gspc::we::priority_type()
      , std::optional<gspc::we::type::eureka_id_type>{}
      , std::list<gspc::we::type::Preference>{}
      , gspc::we::type::track_shared{}
      );
    gspc::we::port_id_type const port_id_eureka_gid
      ( trans_eureka.add_port ( gspc::we::type::Port
                                 ( "id"
                                 , gspc::we::type::port::direction::In{}
                                 , signature::signature_type ("string")
                                 , gspc::we::type::property::type()
                                 )
                               )
      );
    gspc::we::port_id_type const port_id_in
      ( trans_eureka.add_port ( gspc::we::type::Port
                                 ( "in"
                                 , gspc::we::type::port::direction::In{}
                                 , signature::signature_type ("long")
                                 , gspc::we::type::property::type()
                                 )
                               )
      );
    gspc::we::port_id_type const port_id_out
      ( trans_eureka.add_port ( gspc::we::type::Port
                                 ( "out"
                                 , gspc::we::type::port::direction::Out{}
                                 , signature::signature_type ("set")
                                 , gspc::we::type::property::type()
                                 )
                               )
      );

    gspc::we::place_id_type const pid_eureka_gid
      (net.add_place (gspc::we::type::place::type ( "id"
                                  , signature::signature_type ("string")
                                  , true
                                  , std::nullopt
                                  , gspc::we::type::property::type{}
                                  , gspc::we::type::place::type::Generator::No{}
                                  )
                     )
      );

    gspc::we::type::property::type empty;
    gspc::we::transition_id_type const tid (net.add_transition (trans_eureka));

    net.add_connection ( gspc::we::edge::PT_READ{}
                       , tid
                       , pid_eureka_gid
                       , port_id_eureka_gid
                       , empty
                       );
    net.add_connection (gspc::we::edge::PT{}, tid, pid_in, port_id_in, empty);
    net.add_eureka (tid, port_id_out);

    net.put_value (pid_eureka_gid, eureka_id_);
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
       auto const n_tokens ( gspc::testing::random<size_t>{}()
                           % MAX_TOKENS
                           + 2
                           );
       tokens = gspc::testing::unique_randoms
                <std::vector<long>> (n_tokens);

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

  std::optional<gspc::we::type::eureka_id_type> eureka_received;
  std::mt19937 random_engine
    (gspc::testing::detail::GLOBAL_random_engine()());

  BOOST_REQUIRE
    ( !eureka_net.net.fire_expressions_and_extract_activity_random_TESTING_ONLY
        ( random_engine
        , [] ( gspc::pnet::type::value::value_type const&
             , gspc::pnet::type::value::value_type const&
             )
          {
            throw std::logic_error ("got workflow_response unsupported");
          }
        , [&eureka_received] (gspc::we::type::eureka_ids_type const& ids)
          {
            BOOST_TEST (ids.size() == 1);
            eureka_received = *ids.begin();
          }
        )
    );

  BOOST_REQUIRE_EQUAL ( eureka_received
                      , std::nullopt
                      );
}

BOOST_AUTO_TEST_CASE (check_transition_generates_eureka_id_on_eureka_response)
{
  std::vector<long> tokens;

  long const eureka_value
    ([&tokens]
     {
       auto const n_tokens ( gspc::testing::random<size_t>{}()
                           % MAX_TOKENS
                           + 1
                           );
       tokens = gspc::testing::unique_randoms
                <std::vector<long>> (n_tokens);
       return tokens.at ( gspc::testing::random<std::vector<long>::size_type>{}()
                        % n_tokens
                        );
     }()
    );

  net_with_eureka_transition eureka_net
    ( eureka_id
    , "${in} :eq: " + std::to_string (eureka_value) + "L"
    );
  eureka_net.put_tokens (tokens);

  std::optional<gspc::we::type::eureka_id_type> eureka_received;
  std::mt19937 random_engine
    (gspc::testing::detail::GLOBAL_random_engine()());

  BOOST_REQUIRE
    ( !eureka_net.net.fire_expressions_and_extract_activity_random_TESTING_ONLY
        ( random_engine
        , [] ( gspc::pnet::type::value::value_type const&
             , gspc::pnet::type::value::value_type const&
             )
          {
            throw std::logic_error ("got workflow_response unsupported");
          }
        , [&eureka_received] (gspc::we::type::eureka_ids_type const& ids)
          {
            BOOST_TEST (ids.size() == 1);
            eureka_received = *ids.begin();
          }
        )
    );

  BOOST_REQUIRE_EQUAL ( eureka_received
                      , std::optional<gspc::we::type::eureka_id_type> (eureka_id)
                      );
}

BOOST_AUTO_TEST_CASE (check_transition_generates_one_or_more_eureka_responses)
{
  size_t num_eureka_expected = 0;
  std::vector<long> tokens;

  long const eureka_value
    ([&tokens, &num_eureka_expected]
     {
       auto const n_tokens ( gspc::testing::random<size_t>{}()
                           % MAX_TOKENS
                           + 1
                           );
       tokens = gspc::testing::unique_randoms
                <std::vector<long>> (n_tokens);
       auto const eureka
        (tokens.at (gspc::testing::random<std::vector<long>::size_type>{}() % n_tokens));
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
    (gspc::testing::detail::GLOBAL_random_engine()());

  BOOST_REQUIRE
    ( !eureka_net.net.fire_expressions_and_extract_activity_random_TESTING_ONLY
        ( random_engine
        , [] ( gspc::pnet::type::value::value_type const&
             , gspc::pnet::type::value::value_type const&
             )
          {
            throw std::logic_error ("got workflow_response unsupported");
          }
        , [&num_eureka_received] (gspc::we::type::eureka_ids_type const& ids)
          {
            BOOST_TEST (ids.size() == 1);
            num_eureka_received++;
          }
        )
    );

  BOOST_REQUIRE_EQUAL (num_eureka_expected, num_eureka_received);
}
