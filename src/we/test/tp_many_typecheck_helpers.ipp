// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <we/signature_of.hpp>
#include <we/type/value/name_of.hpp>
#include <we/type/value/wrap.hpp>

#include <we/test/net.common.hpp>

#include <util-generic/testing/random.hpp>

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

FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE (value::structured_type)
{
  //! \note Tests below on every structure having the same signature.
  value::structured_type m;
  m.emplace_back ("first", fhg::util::testing::random<long>{}());
  m.emplace_back ("second", fhg::util::testing::random<long>{}());
  return m;
}

//! \todo Have all container randoms contain random types inside?
using value_map = std::map<value::value_type, value::value_type>;
FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE (value_map)
{
  std::vector<unsigned long> const values
    { fhg::util::testing::randoms<std::vector<unsigned long>>
        (fhg::util::testing::random<size_t>{}() % NUM_ITEMS_IN_SUB_LIST)
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

FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE (std::list<value::value_type>)
{
  std::vector<unsigned long> const values
    { fhg::util::testing::randoms<std::vector<unsigned long>>
        (fhg::util::testing::random<size_t>{}() % NUM_ITEMS_IN_SUB_LIST)
    };

  return value::wrap (std::list<unsigned long> (values.begin(), values.end()));
}

FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE (std::set<value::value_type>)
{
  std::vector<unsigned long> const values
    { fhg::util::testing::randoms<std::vector<unsigned long>>
        (fhg::util::testing::unique_random<size_t>{}() % NUM_ITEMS_IN_SUB_LIST)
    };

  return value::wrap (std::set<unsigned long> (values.begin(), values.end()));
}

FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE (bitsetofint::type)
{
  bitsetofint::type bs;
  bs.push_back (fhg::util::testing::random<unsigned long>{}());
  return bs;
}

FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE (we::type::bytearray)
{
  return we::type::bytearray (fhg::util::testing::random<unsigned long>{}());
}

FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE (we::type::literal::control)
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
    return pnet::signature_of (_generate());
  }

  std::list<value::value_type> value_generator::list() const
  {
    constexpr auto const MAX_PUT_LIST = 1000;

    std::list<value::value_type> list;
    std::generate_n
      ( std::back_inserter (list)
      , 1 + (fhg::util::testing::random<size_t>{}() % MAX_PUT_LIST)
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
      void operator() (T const& t) const
    {
      std::function<value::value_type()> generate
        ([] { return fhg::util::testing::random<T>{}(); });
      result->emplace (value::name_of (t), std::move (generate));
    }
  };
  type_name_to_generator make_type_name_to_generator()
  {
    type_name_to_generator result;
    type_wrapper const visitor {&result};
    boost::mpl::for_each<value::value_type::types> (visitor);
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

  net_with_empty_transition_with_tp_many::net_with_empty_transition_with_tp_many
      (std::string out_type_str)
    : pid_in (net.add_place (place::type ("in", "list", boost::none)))
    , pid_tp_many ( net.add_place ( place::type ( get_tp_many_place_name()
                                                , name_to_signature (out_type_str)
                                                , boost::none
                                                )
                                  )
                  )
    , pid_out ( net.add_place ( place::type ( "out"
                                            , name_to_signature (out_type_str)
                                            , boost::none
                                            )
                              )
              )
  {
    we::type::Transition trans_io
      ( "put_many_list_typecheck"
      , we::type::Expression ("")
      , boost::none
      , no_properties()
      , we::priority_type()
      , boost::optional<we::type::eureka_id_type>{}
      , std::list<we::type::Preference>{}
      );

    we::port_id_type const port_id_in
      ( trans_io.add_port
          (we::type::Port ("in", we::type::port::direction::In{}, "list", we::type::property::type{}))
      );
    we::port_id_type const port_id_out
      ( trans_io.add_port
          (we::type::Port ("in", we::type::port::direction::Out{}, "list", we::type::property::type{}))
      );

    we::transition_id_type const tid (net.add_transition (trans_io));
    net.add_connection (we::edge::PT, tid, pid_in, port_id_in, {});
    net.add_connection (we::edge::TP_MANY, tid, pid_tp_many, port_id_out, {});

    we::type::Transition trans_out
      ( "put_many_list_out"
      , we::type::Expression ("")
      , boost::none
      , no_properties()
      , we::priority_type()
      , boost::optional<we::type::eureka_id_type>{}
      , std::list<we::type::Preference>{}
      );

    signature::signature_type const& type = name_to_signature (out_type_str);
    we::port_id_type const pport_id_in
      ( trans_out.add_port
          (we::type::Port ("out", we::type::port::direction::In{}, type, we::type::property::type{}))
      );
    we::port_id_type const pport_id_out
      ( trans_out.add_port
          (we::type::Port ("out", we::type::port::direction::Out{}, type, we::type::property::type{}))
      );

    we::transition_id_type const tid_out (net.add_transition (trans_out));
    net.add_connection (we::edge::PT, tid_out, pid_tp_many, pport_id_in, {});
    net.add_connection (we::edge::TP, tid_out, pid_out, pport_id_out, {});
  }

  std::list<value::value_type> net_with_empty_transition_with_tp_many
    ::get_list_of_output_tokens() const
  {
    std::list<value::value_type> out_list;
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
}
