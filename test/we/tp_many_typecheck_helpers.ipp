// Copyright (C) 2019-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/signature_of.hpp>
#include <gspc/we/type/shared.hpp>
#include <gspc/we/type/value/name_of.hpp>
#include <gspc/we/type/value/wrap.hpp>

#include <test/we/net.common.hpp>

#include <gspc/testing/random.hpp>
#include <gspc/testing/random/cpp_int.hpp>

#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
  constexpr auto const NUM_ITEMS_IN_SUB_LIST = 10;
  const std::string tp_many_place_name ("out_tp_many");
}

GSPC_TESTING_RANDOM_SPECIALIZE_SIMPLE (value::structured_type)
{
  //! \note Tests below on every structure having the same signature.
  value::structured_type m;
  m.emplace_back ("first", gspc::testing::random<long>{}());
  m.emplace_back ("second", gspc::testing::random<long>{}());
  return m;
}

//! \todo Have all container randoms contain random types inside?
using value_map = std::map<value::value_type, value::value_type>;
GSPC_TESTING_RANDOM_SPECIALIZE_SIMPLE (value_map)
{
  std::vector<unsigned long> const values
    { gspc::testing::randoms<std::vector<unsigned long>>
        (gspc::testing::random<size_t>{}() % NUM_ITEMS_IN_SUB_LIST)
    };
  std::map<unsigned long, unsigned long> mapped_values;
  std::transform ( values.begin()
                 , values.end()
                 , std::inserter (mapped_values, mapped_values.end())
                 , [] (unsigned long s)
                   {
                     return std::make_pair (s, 1);
                   }
                 );

  return value::wrap (mapped_values);
}

GSPC_TESTING_RANDOM_SPECIALIZE_SIMPLE (std::list<value::value_type>)
{
  std::vector<unsigned long> const values
    { gspc::testing::randoms<std::vector<unsigned long>>
        (gspc::testing::random<size_t>{}() % NUM_ITEMS_IN_SUB_LIST)
    };

  return value::wrap (std::list<unsigned long> (values.begin(), values.end()));
}

GSPC_TESTING_RANDOM_SPECIALIZE_SIMPLE (std::set<value::value_type>)
{
  std::vector<unsigned long> const values
    { gspc::testing::randoms<std::vector<unsigned long>>
        (gspc::testing::unique_random<size_t>{}() % NUM_ITEMS_IN_SUB_LIST)
    };

  return value::wrap (std::set<unsigned long> (values.begin(), values.end()));
}

GSPC_TESTING_RANDOM_SPECIALIZE_SIMPLE (gspc::pnet::type::bitsetofint::type)
{
  gspc::pnet::type::bitsetofint::type bs;
  bs.push_back (gspc::testing::random<unsigned long>{}());
  return bs;
}

GSPC_TESTING_RANDOM_SPECIALIZE_SIMPLE (::gspc::we::type::bytearray)
{
  return ::gspc::we::type::bytearray (gspc::testing::random<unsigned long>{}());
}

GSPC_TESTING_RANDOM_SPECIALIZE_SIMPLE (::gspc::we::type::shared)
{
  // Use unique_random to ensure no duplicate (value, cleanup_place) pairs,
  // which would cause "already exists" errors in the reference counting.
  static gspc::testing::unique_random<std::string> unique_string;
  return ::gspc::we::type::shared
    ( unique_string()
    , "cleanup"
    );
}

GSPC_TESTING_RANDOM_SPECIALIZE_SIMPLE (::gspc::we::type::literal::control)
{
  return {};
}

namespace
{
  value_generator::value_generator (decltype (_generate) generate)
    : _generate (std::move (generate))
  {}

  signature::signature_type value_generator::signature_of_single() const
  {
    return gspc::pnet::signature_of (_generate());
  }

  std::list<value::value_type> value_generator::list() const
  {
    constexpr auto const MAX_PUT_LIST = 1000;

    std::list<value::value_type> list;
    std::generate_n
      ( std::back_inserter (list)
      , 1 + (gspc::testing::random<size_t>{}() % MAX_PUT_LIST)
      , _generate
      );
    return list;
  }

  using type_name_to_generator
    = std::unordered_map<std::string, value_generator>;
  struct type_wrapper
  {
    type_name_to_generator* result;
    template<typename T>
      void operator() (T const&) const
    {
      std::function<value::value_type()> generate
        {[] { return gspc::testing::random<T>{}(); }};
      result->emplace (value::name_of (T{}), std::move (generate));
    }
  };
  type_name_to_generator make_type_name_to_generator()
  {
    type_name_to_generator result;
    type_wrapper const visitor {&result};
    ::boost::mpl::for_each<value::value_type::types> (visitor);
    return result;
  }

  auto const type_name_to_generators (make_type_name_to_generator());
  signature::signature_type name_to_signature (std::string name)
  {
    return type_name_to_generators.at (name).signature_of_single();
  }
  std::list<value::value_type> make_list_of_values (std::string type_str)
  {
    return type_name_to_generators.at (type_str).list();
  }
}

net_with_empty_transition_with_tp_many::net_with_empty_transition_with_tp_many
    (std::string out_type_str)
  : pid_in
      ( net.add_place
          ( gspc::we::type::place::type
              ( "in"
              , "list"
              , std::nullopt
              , std::nullopt
              , gspc::we::type::property::type{}
              , gspc::we::type::place::type::Generator::No{}
              )
          )
      )
  , pid_tp_many ( net.add_place ( gspc::we::type::place::type ( get_tp_many_place_name()
                                              , name_to_signature (out_type_str)
                                              , std::nullopt
                                              , std::nullopt
                                              , gspc::we::type::property::type{}
                                              , gspc::we::type::place::type::Generator::No{}
                                              )
                                )
                )
  , pid_out ( net.add_place ( gspc::we::type::place::type ( "out"
                                          , name_to_signature (out_type_str)
                                          , std::nullopt
                                          , std::nullopt
                                          , gspc::we::type::property::type{}
                                          , gspc::we::type::place::type::Generator::No{}
                                          )
                            )
            )
{
  // Add a shared_sink cleanup place for tests involving `shared`
  // type values.  The random generator for `gspc::we::type::shared` uses
  // "cleanup" as the place.
  net.add_place
    ( gspc::we::type::place::type
      { "cleanup"
      , "shared"
      , std::nullopt
      , true /* shared_sink */
      , gspc::we::type::property::type{}
      , gspc::we::type::place::type::Generator::No{}
      }
    );

  gspc::we::type::Transition trans_io
    ( "put_many_list_typecheck"
    , gspc::we::type::Expression ("")
    , std::nullopt
    , no_properties()
    , gspc::we::priority_type()
    , std::optional<gspc::we::type::eureka_id_type>{}
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  gspc::we::port_id_type const port_id_in
    ( trans_io.add_port
        (gspc::we::type::Port ("in", gspc::we::type::port::direction::In{}, "list", gspc::we::type::property::type{}))
    );
  gspc::we::port_id_type const port_id_out
    ( trans_io.add_port
        (gspc::we::type::Port ("in", gspc::we::type::port::direction::Out{}, "list", gspc::we::type::property::type{}))
    );

  gspc::we::transition_id_type const tid (net.add_transition (trans_io));
  net.add_connection (gspc::we::edge::PT{}, tid, pid_in, port_id_in, {});
  net.add_connection (gspc::we::edge::TP_MANY{}, tid, pid_tp_many, port_id_out, {});

  gspc::we::type::Transition trans_out
    ( "put_many_list_out"
    , gspc::we::type::Expression ("")
    , std::nullopt
    , no_properties()
    , gspc::we::priority_type()
    , std::optional<gspc::we::type::eureka_id_type>{}
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  gspc::pnet::type::signature::signature_type const& type = name_to_signature (out_type_str);
  gspc::we::port_id_type const pport_id_in
    ( trans_out.add_port
        (gspc::we::type::Port ("out", gspc::we::type::port::direction::In{}, type, gspc::we::type::property::type{}))
    );
  gspc::we::port_id_type const pport_id_out
    ( trans_out.add_port
        (gspc::we::type::Port ("out", gspc::we::type::port::direction::Out{}, type, gspc::we::type::property::type{}))
    );

  gspc::we::transition_id_type const tid_out (net.add_transition (trans_out));
  net.add_connection (gspc::we::edge::PT{}, tid_out, pid_tp_many, pport_id_in, {});
  net.add_connection (gspc::we::edge::TP{}, tid_out, pid_out, pport_id_out, {});
}

std::list<gspc::pnet::type::value::value_type> net_with_empty_transition_with_tp_many
  ::get_list_of_output_tokens() const
{
  std::list<gspc::pnet::type::value::value_type> out_list;
  for (auto token : net.get_token (pid_out))
  {
    out_list.push_back (token.second);
  }
  return out_list;
}

std::string const& net_with_empty_transition_with_tp_many
    ::get_tp_many_place_name () const
  {
    return tp_many_place_name;
  }
