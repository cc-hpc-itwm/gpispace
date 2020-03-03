#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/optional/optional_io.hpp>

#include <we/type/transition.hpp>
#include <we/type/net.hpp>
#include <we/type/signature.hpp>
#include <we/type/value.hpp>
#include <we/type/value/wrap.hpp>

#include <util-generic/testing/random.hpp>

namespace
{
  namespace signature = pnet::type::signature;
  namespace value = pnet::type::value;

  we::type::heureka_id_type const heureka_id ("group");

  size_t const MAX_TOKENS = 1000;
  long const MAX_TOKEN_VALUE = 100;

  struct net_with_heureka_transition
  {
    std::string const heureka_condition;
    we::type::net_type net;
    we::place_id_type pid_in;

    net_with_heureka_transition ( we::type::heureka_id_type const id
                                , std::string condition
                                );
    void put_tokens (std::vector<long> const&);
  };

  net_with_heureka_transition::net_with_heureka_transition
      ( we::type::heureka_id_type const h_id
      , std::string condition
      )
    : heureka_condition (condition)
    , pid_in
      (net.add_place (place::type ( "in"
                                  , signature::signature_type ("long")
                                  , true
                                  )
                     )
      )
  {
    we::type::transition_t trans_heureka
      ( "generate_ids_on_heureka_port"
        , we::type::expression_t ("${out} := set_insert (Set{}, ${id})")
        , we::type::expression_t (heureka_condition)
        , {}
        , we::priority_type()
      );
    we::port_id_type const port_id_hid
      ( trans_heureka.add_port ( we::type::port_t
                                 ( "id"
                                 , we::type::PORT_IN
                                 , signature::signature_type ("string")
                                 , we::type::property::type()
                                 )
                               )
      );
    we::port_id_type const port_id_in
      ( trans_heureka.add_port ( we::type::port_t
                                 ( "in"
                                 , we::type::PORT_IN
                                 , signature::signature_type ("long")
                                 , we::type::property::type()
                                 )
                               )
      );
    we::port_id_type const port_id_out
      ( trans_heureka.add_port ( we::type::port_t
                                 ( "out"
                                 , we::type::PORT_OUT
                                 , signature::signature_type ("set")
                                 , we::type::property::type()
                                 )
                               )
      );

    we::place_id_type const pid_hid
      (net.add_place (place::type ( "id"
                                  , signature::signature_type ("string")
                                  , true
                                  )
                     )
      );

    we::type::property::type empty;
    we::transition_id_type const tid (net.add_transition (trans_heureka));

    net.add_connection (we::edge::PT_READ, tid, pid_hid, port_id_hid, empty);
    net.add_connection (we::edge::PT, tid, pid_in, port_id_in, empty);
    net.add_heureka (tid, port_id_out);

    net.put_value (pid_hid, h_id);
  }

  void net_with_heureka_transition::put_tokens
    (std::vector<long> const& values)
  {
    for (auto const& v : values)
    {
      net.put_value (pid_in, v);
    }
  }
}

BOOST_DATA_TEST_CASE ( check_transition_generates_heureka_id_on_heureka_responses
                     , std::vector<bool> ({true, false})
                     , to_heureka
                     )
{
  std::vector<long> tokens;

  long const heureka_value
    ([&to_heureka, &tokens]
     {
       size_t const n_tokens = ( fhg::util::testing::unique_random<size_t>{}()
                               % MAX_TOKENS
                               ) + 1;
       tokens = fhg::util::testing::randoms < std::vector<long>
                                            , fhg::util::testing::unique_random
                                            > (n_tokens);
       if (to_heureka)
       {
         return tokens.at ( fhg::util::testing::unique_random<long>{}()
                          % n_tokens
                          );
       }
       else
       {
         long _value = tokens.back();
         tokens.pop_back ();

         return _value;
       }
     }()
    );

  net_with_heureka_transition heureka_net
    ( heureka_id
    , "${in} :eq: " + std::to_string (heureka_value) + "L"
    );
  heureka_net.put_tokens (tokens);

  std::mt19937 _random_engine;
  boost::optional<we::type::heureka_id_type> heureka_received;

  BOOST_REQUIRE
    ( !heureka_net.net.fire_expressions_and_extract_activity_random
        ( _random_engine
        , [] ( pnet::type::value::value_type const&
             , pnet::type::value::value_type const&
             )
          {
            throw std::logic_error ("got workflow_response unsupported");
          }
        , [&heureka_received] (we::type::heureka_ids_type const& ids)
          {
            if (ids.size() == 1)
            {
              heureka_received
                = boost::optional<we::type::heureka_id_type> (*ids.begin());
            }
            else if (ids.size())
            {
              throw std::logic_error ("more than one heureka id");
            }
          }
        )
    );

  boost::optional<we::type::heureka_id_type> heureka_expected =
    to_heureka ? boost::optional<we::type::heureka_id_type> (heureka_id)
               : boost::none;

  BOOST_REQUIRE_EQUAL ( heureka_expected
                      , heureka_received
                      );
}

BOOST_AUTO_TEST_CASE (check_transition_generates_one_or_more_heureka_responses)
{
  size_t num_heureka_expected = 0;
  std::vector<long> tokens;

  long const heureka_value
    ([&tokens, &num_heureka_expected]
     {
       size_t const n_tokens = ( fhg::util::testing::unique_random<size_t>{}()
                               % MAX_TOKENS
                               ) + 1;
       tokens = fhg::util::testing::randoms < std::vector<long>
                                            , fhg::util::testing::unique_random
                                            > (n_tokens);
       long heureka = tokens.at ( fhg::util::testing::unique_random<long>{}()
                                % n_tokens
                                );
       std::for_each ( tokens.begin()
                     , tokens.end()
                     , [&heureka, &num_heureka_expected] (long const& val)
                       {
                         if (val >= heureka) num_heureka_expected++;
                       }
                     );

       return heureka;
     }()
    );

  net_with_heureka_transition heureka_net
    ( heureka_id
    , "${in} :ge: " + std::to_string (heureka_value) + "L"
    );
  heureka_net.put_tokens (tokens);

  std::mt19937 _random_engine;
  size_t num_heureka_received = 0;

  BOOST_REQUIRE
    ( !heureka_net.net.fire_expressions_and_extract_activity_random
        ( _random_engine
        , [] ( pnet::type::value::value_type const&
             , pnet::type::value::value_type const&
             )
          {
            throw std::logic_error ("got workflow_response unsupported");
          }
        , [&num_heureka_received] (we::type::heureka_ids_type const& ids)
          {
            if (ids.size() == 1)
            {
              num_heureka_received++;
            }
            else if (ids.size())
            {
              throw std::logic_error ("more than one heureka id");
            }
          }
        )
    );

  BOOST_REQUIRE_EQUAL (num_heureka_expected, num_heureka_received);
}
