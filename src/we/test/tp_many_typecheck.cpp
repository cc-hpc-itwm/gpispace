#include <boost/test/unit_test.hpp>

#include <we/layer.hpp>
#include <we/signature_of.hpp>
#include <we/type/value/show.hpp>
#include <we/type/signature/show.hpp>
#include <we/type/activity.hpp>
#include <we/test/operator_equal.hpp>
#include <we/type/expression.hpp>
#include <we/type/module_call.hpp>
#include <we/type/value/wrap.hpp> 
#include <we/type/value/unwrap.hpp> 
#include <we/type/signature/names.hpp>
#include <we/type/signature.hpp>
#include <we/type/value/name_of.hpp>
#include <we/type/transition.hpp>
#include <we/test/net.common.hpp>

#include <util-generic/serialization/exception.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/any.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/output_test_stream.hpp>

#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <tuple>
#include <iterator>
#include <map>
#include <vector>

namespace {
  namespace value = pnet::type::value;
  namespace signature = pnet::type::signature; 

  static const size_t MAX_PUT_LIST = 1000;
  static const size_t NUM_ITEMS_IN_SUB_LIST = 10;

  static std::unordered_map <std::string, value::value_type> types_map;
  static std::vector <std::string> types_str_vec; 

  value::structured_type get_user_struct_elem () 
  { 
    value::structured_type m;
    m.push_back (std::make_pair ( std::string ("first")  
                                , fhg::util::testing::random<long>{}()
                                )
                );
    m.push_back (std::make_pair ( std::string ("second")  
                                , fhg::util::testing::random<long>{}()
                                )
                );

    return m; 
  }

  void create_empty_transition_with_tp_many 
    ( we::type::net_type & net
    , we::place_id_type & pid_in
    , we::place_id_type & pid_out
    , const signature::signature_type & out_place_type 
    ) 
  {
    we::type::property::type empty;

    we::type::transition_t trans_io
      ( "put_many_list_typecheck"
        , we::type::expression_t ("")
        , boost::none
        , no_properties()
        , we::priority_type()
      );

    pid_in = net.add_place (place::type ("in", std::string ("list"), boost::none));

    pid_out = net.add_place (place::type ("out", out_place_type, boost::none));

    we::port_id_type const port_id_in
      ( trans_io.add_port
        (we::type::port_t ("in", we::type::PORT_IN, std::string("list")))
      );
    we::port_id_type const port_id_out
      ( trans_io.add_port
        (we::type::port_t ("in", we::type::PORT_OUT, std::string("list")))
      );

    we::transition_id_type const tid (net.add_transition (trans_io));

    net.add_connection (we::edge::PT, tid, pid_in, port_id_in, empty);
    net.add_connection (we::edge::TP_MANY, tid, pid_out, port_id_out, empty);
  }


  template <typename T>
    void get_sorted_list_of_output_tokens ( const we::type::net_type & net
                                          , const we::place_id_type & pid_out
                                          , std::list<T> & out_list
                                          )
    {
      for (auto token : net.get_token(pid_out)) 
      {
        out_list.push_back (value::from_value<T> (token.second)); 
      }

      out_list.sort(); 
    }

  class visitor_addv_and_grow_list : public boost::static_visitor<void>
  {
    private: 
      std::list <value::value_type> & list;
      size_t max; 

      size_t get_n_to_grow_list () const
      {
        return max ? (fhg::util::testing::random<size_t>{}() % max) : 0; 
      }

    public:
      visitor_addv_and_grow_list ( std::list<value::value_type> & _list
                                    , size_t _max
                                    ) : list (_list), max (_max) {}

      void operator () (value::structured_type val) const 
      {
        list.emplace_back (val); 

        for (size_t i (0); i < get_n_to_grow_list(); ++i)
        {
          list.emplace_back (get_user_struct_elem ()); 
        }
      }

      void operator () (std::map<value::value_type, value::value_type> val) const
      {
        list.emplace_back (val); 

        for (size_t i (0); i < get_n_to_grow_list(); ++i)
        {
          std::vector<unsigned long> const values
            {fhg::util::testing::randoms<std::vector<unsigned long>> 
             (fhg::util::testing::random<size_t>{}() % NUM_ITEMS_IN_SUB_LIST)
            };
          std::map<unsigned long, unsigned long> mapped_values; 
          std::transform ( values.begin()
                         , values.end()
                         , std::inserter (mapped_values, mapped_values.end())
                         , [](const unsigned long &s) 
                            { 
                              return std::make_pair(s, 1); 
                            }
                         ); 

          list.emplace_back (value::wrap (mapped_values));
        }
      }

      void operator () (std::list<value::value_type> val) const
      {
        list.emplace_back (val); 

        for (size_t i (0); i < get_n_to_grow_list(); ++i)
        {
          std::vector<unsigned long> const values
            {fhg::util::testing::randoms<std::vector<unsigned long>> 
             (fhg::util::testing::random<size_t>{}() % NUM_ITEMS_IN_SUB_LIST)
            };
          std::list<unsigned long> value (values.begin(), values.end()); 
          
          list.emplace_back (value::wrap (value));
        }
      }

      void operator () (std::set<value::value_type> val)  const
      {
        list.emplace_back (val); 

        for (size_t i (0); i < get_n_to_grow_list(); ++i)
        {
          std::vector<unsigned long> const values
            {fhg::util::testing::randoms<std::vector<unsigned long>> 
             (fhg::util::testing::random<size_t>{}() % NUM_ITEMS_IN_SUB_LIST)
            };
          std::set<unsigned long> value (values.begin(), values.end()); 

          list.emplace_back (value::wrap (value));
        }
      }
      
      void operator () (bitsetofint::type val) const
      {
        list.emplace_back (val); 

        for (size_t i (0); i < get_n_to_grow_list(); ++i)
        {
          bitsetofint::type bs;
          bs.push_back (fhg::util::testing::random<unsigned long>{}()); 

          list.emplace_back (bs); 
        }
      }
      
      void operator () (we::type::bytearray val) const
      {
        list.emplace_back (val); 

        for (size_t i (0); i < get_n_to_grow_list(); ++i)
        {
          unsigned long value = fhg::util::testing::random<unsigned long>{}(); 
          const we::type::bytearray x (&value);

          list.emplace_back (x); 
        }
      }

      void operator () (we::type::literal::control val) const
      {
        list.emplace_back (val); 

        for (size_t i (0); i < get_n_to_grow_list(); ++i)
        {
          list.emplace_back (val); 
        }
      }

      void operator () (std::string val) const
      {
        list.emplace_back (val); 

        for (size_t i (0); i < get_n_to_grow_list(); ++i)
        {
          list.emplace_back (fhg::util::testing::random<std::string>{}()); 
        }
      }

      template <typename T>
        void operator () (T val) const
        {
          list.emplace_back (val); 

          for (size_t i (0); i < get_n_to_grow_list(); ++i)
          {
            unsigned long value = fhg::util::testing::random<unsigned long>{}(); 
            list.emplace_back ((T) (value));
          }
        }
  };

  struct type_wrapper {
    template<typename T> void operator()(T t) 
    {
      value::value_type v (t); 
      std::string type_str (value::name_of (t)); 
        
      if (type_str == "char") 
      {
        //hack for prematurely terminated exception with 'char = 0'
        types_map[type_str] = ((char) 42); 
      }
      else if (type_str == "struct") 
      {
        //hack to avoid empty struct
        types_map[type_str] = get_user_struct_elem ();
      } 
      else 
      {
        types_map[type_str] = v; 
      }
    }
  };

  bool populate_types_vec_and_map () 
  {
    boost::mpl::for_each<value::value_type::types> (type_wrapper());

    boost::copy ( types_map | boost::adaptors::map_keys
                , std::back_inserter (types_str_vec)
                ); 
    return true;
  }

  const std::vector<std::string> & get_types_vec () 
  { 
    static bool initialized (populate_types_vec_and_map());
    (void) initialized; 

    return types_str_vec; 
  }

  const value::value_type & get_type (std::string type_str) 
  { 
    return types_map [type_str]; 
  }

}

BOOST_DATA_TEST_CASE  ( tp_many_typecheck_match_input_list_and_output_tokens
                      , ::get_types_vec ()
                      , in_type_str
                      )
{
  we::type::net_type net;
  we::place_id_type pid_in;
  we::place_id_type pid_out;
  std::list<::value::value_type> in_list; 
  std::list<::value::value_type> out_tokens; 

  const ::value::value_type & type = ::get_type (in_type_str); 
  ::create_empty_transition_with_tp_many ( net
                                         , pid_in
                                         , pid_out
                                         , pnet::signature_of (type) 
                                         ); 

  boost::apply_visitor (::visitor_addv_and_grow_list ( in_list
                                                     , ::MAX_PUT_LIST
                                                     )
                       , type
                       ); 

  net.put_value (pid_in, value::wrap (in_list)); 
  BOOST_REQUIRE_EQUAL (net.get_token (pid_in).size(), 1);

  BOOST_REQUIRE
  (
    !net.fire_expressions_and_extract_activity_random 
    (random_engine(), unexpected_workflow_response)
  );

  BOOST_REQUIRE_EQUAL ( net.get_token (pid_out).size()
                      , in_list.size()
                      ); 

  in_list.sort(); 
  get_sorted_list_of_output_tokens (net, pid_out, out_tokens); 
  BOOST_REQUIRE (in_list == out_tokens); 
}

BOOST_DATA_TEST_CASE ( tp_many_typecheck_mismatch_expection_for_all_types
                     , boost::unit_test::data::make (::get_types_vec ()) 
                       * boost::unit_test::data::make (::get_types_vec ()) 
                     , in_type_str
                     , out_type_str
                     )
{
  if (out_type_str != in_type_str) 
  { 
    we::type::net_type net;
    we::place_id_type pid_in;
    we::place_id_type pid_out;
    std::list<value::value_type> in_list; 

    const ::value::value_type & in_type = ::get_type (in_type_str); 
    const ::value::value_type & out_type = ::get_type (out_type_str); 

    ::create_empty_transition_with_tp_many 
      ( net
      , pid_in
      , pid_out
      , pnet::signature_of (out_type)
      ); 

    boost::apply_visitor 
      ( ::visitor_addv_and_grow_list (in_list, 0)
      , in_type
      );

    net.put_value (pid_in, value::wrap (in_list)); 
    BOOST_REQUIRE_EQUAL (net.get_token (pid_in).size(), 1);

    fhg::util::testing::require_exception_with_message
      <pnet::exception::type_mismatch> 
      ( [&net] 
        { net.fire_expressions_and_extract_activity_random 
          (random_engine(), unexpected_workflow_response); 
        } 
        , boost::format ("type error: type mismatch"
          " for field '%1%':"
          " expected type '%2%'"
          ", value '%3%' has type '%4%'"
          )
        % "out"
        % ::signature::show (pnet::signature_of (out_type))
        % value::show (in_type) 
        % ::signature::show (pnet::signature_of (in_type))
      );
  }
} 
