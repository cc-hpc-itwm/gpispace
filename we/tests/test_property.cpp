// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_type_property

#include <we/type/property.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/xml.hpp>

#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>

namespace prop = we::type::property;

namespace
{
  class ordered_dump : public boost::static_visitor<void>
  {
  public:
    ordered_dump (std::ostream& s)
      : _s (s)
    { }

    template<typename T> void operator() (const T& t) const
    {
      _s << t;
    }

    void operator() (const prop::type& t) const
    {
      ::fhg::util::xml::xmlstream xs (_s);
      prop::dump::ordered_dump (xs, t);
    }

  private:
    std::ostream& _s;
  };

  std::string dump (const prop::mapped_type& m)
  {
    std::stringstream s;
    boost::apply_visitor (ordered_dump (s), m);
    return s.str();
  }

  const prop::value_type default_value ("<<default>>");

  prop::type p;
}

BOOST_AUTO_TEST_CASE (set_value)
{
  const boost::optional<prop::mapped_type> old
    (p.set ("A.A.A", "value_of (A.A.A)"));

  BOOST_REQUIRE_EQUAL (!!old, false);
  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.A.A)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );
}

BOOST_AUTO_TEST_CASE (get_value)
{
  BOOST_REQUIRE_EQUAL (::dump (p.get ("A.A.A")), "value_of (A.A.A)");
  BOOST_REQUIRE_EQUAL (p.get_val ("A.A.A"), "value_of (A.A.A)");
  const boost::optional<const prop::value_type &> maybe_val
    (p.get_maybe_val ("A.A.A"));
  BOOST_REQUIRE_EQUAL (!!maybe_val, true);
  BOOST_REQUIRE_EQUAL (*maybe_val, "value_of (A.A.A)");
  BOOST_REQUIRE_EQUAL
    (p.get_with_default ("A.A.A", default_value), "value_of (A.A.A)");
}

BOOST_AUTO_TEST_CASE (missing_binding)
{
  BOOST_REQUIRE_THROW (p.get ("A.A.B"), prop::exception::missing_binding);
  BOOST_REQUIRE_THROW
    (p.get_val ("A.A.B"), prop::exception::missing_binding);
  BOOST_REQUIRE_EQUAL (p.get_maybe_val ("A.A.B"), boost::none);
  BOOST_REQUIRE_EQUAL
    (p.get_with_default ("A.A.B", default_value), default_value);
}

BOOST_AUTO_TEST_CASE (not_a_value)
{
  BOOST_REQUIRE_EQUAL ( ::dump (p.get ("A"))
                      , "<properties name=\"A\">\n"
                        "  <property key=\"A\">\n"
                        "    value_of (A.A.A)\n"
                        "  </property>\n"
                        "</properties>\n"
                      );
  BOOST_REQUIRE_THROW (p.get_val ("A"), prop::exception::not_a_val);
  BOOST_REQUIRE_EQUAL (p.get_maybe_val ("A"), boost::none);
  BOOST_REQUIRE_EQUAL
    (p.get_with_default ("A", default_value), default_value);

  BOOST_REQUIRE_EQUAL ( ::dump (p.get ("A.A"))
                      , "<property key=\"A\">\n"
                        "  value_of (A.A.A)\n"
                        "</property>\n"
                      );
  BOOST_REQUIRE_THROW (p.get_val ("A.A"), prop::exception::not_a_val);
  BOOST_REQUIRE_EQUAL (p.get_maybe_val ("A.A"), boost::none);
  BOOST_REQUIRE_EQUAL
    (p.get_with_default ("A.A", default_value), default_value);
}

BOOST_AUTO_TEST_CASE (overwrite_subtree)
{
  {
    const prop::mapped_type old_value (p.get ("A.A"));

    const boost::optional<prop::mapped_type> old
      (p.set ("A.A", "value_of (A.A)"));

    BOOST_REQUIRE_EQUAL (!!old, true);
    BOOST_REQUIRE_EQUAL (::dump (*old), ::dump (old_value));
    BOOST_REQUIRE_EQUAL ( ::dump (p)
                        , "<properties name=\"A\">\n"
                          "  <property key=\"A\">\n"
                          "    value_of (A.A)\n"
                          "  </property>\n"
                          "</properties>\n"
                        );
  }

  {
    const prop::mapped_type old_value (p.get ("A"));

    const boost::optional<prop::mapped_type> old
      (p.set ("A", "value_of (A)"));

    BOOST_REQUIRE_EQUAL (!!old, true);
    BOOST_REQUIRE_EQUAL (::dump (*old), ::dump (old_value));
    BOOST_REQUIRE_EQUAL ( ::dump (p)
                        , "<property key=\"A\">\n"
                          "  value_of (A)\n"
                          "</property>\n"
                        );
  }
}

BOOST_AUTO_TEST_CASE (overwriting_a_whole_tree_does_not_count_as_overwriting)
{
  BOOST_REQUIRE_EQUAL (!!p.set ("A.A.A", "value_of (A.A.A)"), false);
  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.A.A)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );

  BOOST_REQUIRE_EQUAL (!!p.set ("A.A.B", "value_of (A.A.B)"), false);
  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.A.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.A.B)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );

  BOOST_REQUIRE_EQUAL (!!p.set ("A.A.C", "value_of (A.A.C)"), false);
  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.A.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.A.B)\n"
                        "    </property>\n"
                        "    <property key=\"C\">\n"
                        "      value_of (A.A.C)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );
}

BOOST_AUTO_TEST_CASE (a_lot_of_trees)
{
  BOOST_REQUIRE_EQUAL (!!p.set ("A.B.A", "value_of (A.B.A)"), false);
  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.A.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.A.B)\n"
                        "    </property>\n"
                        "    <property key=\"C\">\n"
                        "      value_of (A.A.C)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"B\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.B.A)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );

  BOOST_REQUIRE_EQUAL (!!p.set ("A.B.B", "value_of (A.B.B)"), false);
  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.A.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.A.B)\n"
                        "    </property>\n"
                        "    <property key=\"C\">\n"
                        "      value_of (A.A.C)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"B\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.B.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.B.B)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );

  BOOST_REQUIRE_EQUAL (!!p.set ("A.C.A", "value_of (A.C.A)"), false);
  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.A.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.A.B)\n"
                        "    </property>\n"
                        "    <property key=\"C\">\n"
                        "      value_of (A.A.C)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"B\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.B.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.B.B)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"C\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.C.A)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );

  BOOST_REQUIRE_EQUAL (!!p.set ("A.C.B", "value_of (A.C.B)"), false);
  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.A.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.A.B)\n"
                        "    </property>\n"
                        "    <property key=\"C\">\n"
                        "      value_of (A.A.C)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"B\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.B.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.B.B)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"C\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.C.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.C.B)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );
}

BOOST_AUTO_TEST_CASE (multiple_top_level_trees)
{
  BOOST_REQUIRE_EQUAL (!!p.set ("B.A.A", "value_of (B.A.A)"), false);
  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.A.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.A.B)\n"
                        "    </property>\n"
                        "    <property key=\"C\">\n"
                        "      value_of (A.A.C)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"B\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.B.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.B.B)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"C\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.C.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.C.B)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties><properties name=\"B\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (B.A.A)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );
}

BOOST_AUTO_TEST_CASE (remove_tree)
{
  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.A.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.A.B)\n"
                        "    </property>\n"
                        "    <property key=\"C\">\n"
                        "      value_of (A.A.C)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"B\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.B.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.B.B)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"C\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.C.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.C.B)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties><properties name=\"B\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (B.A.A)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );

  p.del ("A.B");

  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.A.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.A.B)\n"
                        "    </property>\n"
                        "    <property key=\"C\">\n"
                        "      value_of (A.A.C)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"C\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.C.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.C.B)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties><properties name=\"B\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (B.A.A)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );
}

BOOST_AUTO_TEST_CASE (remove_value)
{
  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.A.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.A.B)\n"
                        "    </property>\n"
                        "    <property key=\"C\">\n"
                        "      value_of (A.A.C)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"C\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.C.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.C.B)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties><properties name=\"B\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (B.A.A)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );

  p.del ("A.A.A");

  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.A.B)\n"
                        "    </property>\n"
                        "    <property key=\"C\">\n"
                        "      value_of (A.A.C)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"C\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.C.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.C.B)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties><properties name=\"B\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (B.A.A)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );
}

BOOST_AUTO_TEST_CASE (get_value_second_top_level_tree)
{
  BOOST_REQUIRE_EQUAL (::dump (p.get ("B.A.A")), "value_of (B.A.A)");
  BOOST_REQUIRE_EQUAL (p.get_val ("B.A.A"), "value_of (B.A.A)");
  const boost::optional<const prop::value_type &> maybe_val
    (p.get_maybe_val ("B.A.A"));
  BOOST_REQUIRE_EQUAL (!!maybe_val, true);
  BOOST_REQUIRE_EQUAL (*maybe_val, "value_of (B.A.A)");
  BOOST_REQUIRE_EQUAL
    (p.get_with_default ("B.A.A", default_value), "value_of (B.A.A)");
}

BOOST_AUTO_TEST_CASE (get_subtree_second_top_level_tree)
{
  BOOST_REQUIRE_EQUAL ( ::dump (p.get ("B.A"))
                      , "<property key=\"A\">\n"
                        "  value_of (B.A.A)\n"
                        "</property>\n"
                      );
  BOOST_REQUIRE_THROW (p.get_val ("B.A"), prop::exception::not_a_val);
  BOOST_REQUIRE_EQUAL (p.get_maybe_val ("B.A"), boost::none);
  BOOST_REQUIRE_EQUAL
    (p.get_with_default ("B.A", default_value), default_value);
}

BOOST_AUTO_TEST_CASE (overwrite_tree_with_value)
{
  BOOST_REQUIRE_EQUAL (!!p.set ("A.A.B.A", "value_of (A.A.B.A)"), false);
  BOOST_REQUIRE_EQUAL ( ::dump (p)
                      , "<properties name=\"A\">\n"
                        "  <properties name=\"A\">\n"
                        "    <properties name=\"B\">\n"
                        "      <property key=\"A\">\n"
                        "        value_of (A.A.B.A)\n"
                        "      </property>\n"
                        "    </properties>\n"
                        "    <property key=\"C\">\n"
                        "      value_of (A.A.C)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "  <properties name=\"C\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (A.C.A)\n"
                        "    </property>\n"
                        "    <property key=\"B\">\n"
                        "      value_of (A.C.B)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties><properties name=\"B\">\n"
                        "  <properties name=\"A\">\n"
                        "    <property key=\"A\">\n"
                        "      value_of (B.A.A)\n"
                        "    </property>\n"
                        "  </properties>\n"
                        "</properties>\n"
                      );

}

namespace boost
{
  namespace test_tools
  {
    template<> struct print_log_value<prop::path_type>
    {
      void operator()( std::ostream& ostr, const prop::path_type& p)
      {
        ostr << fhg::util::join (p, ".");
      }
    };
  }
}

typedef std::vector<std::pair<std::vector<std::string>, std::string> >
  correct_type;

BOOST_TEST_DONT_PRINT_LOG_VALUE (correct_type::iterator);

BOOST_AUTO_TEST_CASE (visit_all_leafs)
{
  prop::traverse::stack_type stack (prop::traverse::dfs (p));

  // prop::traverse::stack_type correct;
  //! \note This is not an ordered traversal, thus don't check with a stack.
  correct_type correct;

#define VAL(STR)                                                \
  correct.push_back ( std::make_pair ( prop::util::split (STR)  \
                                     , "value_of (" STR ")"     \
                                     )                          \
                    )

  VAL ("B.A.A");
  VAL ("A.C.B");
  VAL ("A.C.A");
  VAL ("A.A.C");
  VAL ("A.A.B.A");

#undef VAL

  BOOST_REQUIRE_EQUAL (stack.size(), correct.size());

  while (!stack.empty())
  {
    const prop::traverse::pair_type elem (stack.top());
    // const prop::traverse::pair_type elem_req (correct.top());
    correct_type::iterator it (std::find (correct.begin(), correct.end(), elem));

    // BOOST_REQUIRE_EQUAL (elem.first, elem_req.first);
    // BOOST_REQUIRE_EQUAL (elem.second, elem_req.second);
    BOOST_REQUIRE_NE (it, correct.end());
    BOOST_REQUIRE_EQUAL (elem.first, it->first);
    BOOST_REQUIRE_EQUAL (elem.second, it->second);

    stack.pop();
    // correct.pop();
    correct.erase (it);
  }
}

namespace boost
{
  namespace test_tools
  {
    template<typename T> struct print_log_value<std::vector<T> >
    {
      void operator()( std::ostream& ostr, const std::vector<T>& p)
      {
        ostr << fhg::util::join (p, ",", "{", "}");
      }
    };
    template<typename T> struct print_log_value<std::list<T> >
    {
      void operator()( std::ostream& ostr, const std::list<T>& p)
      {
        ostr << fhg::util::join (p, ",", "{", "}");
      }
    };
  }
}

BOOST_AUTO_TEST_CASE (store_int_vector)
{
  typedef std::vector<int> vector_type;

  int values[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  vector_type orig (values, values + sizeof (values)/sizeof (*values));

  std::string where ("int_vec");

  p.set_container (where, orig);

  BOOST_REQUIRE_EQUAL (orig, p.get_container<vector_type> (where));
}

struct point
{
  int _x, _y;

  point (int x, int y) : _x (x), _y (y) { }
  bool operator== (const point& o) const { return _x == o._x && _y == o._y; }
};

std::ostream& operator<< (std::ostream& s, const point& p)
{
  s << "(" << p._x << ", " << p._y << ")";
  return s;
}

namespace we
{
  namespace type
  {
    namespace property
    {
      template<> void store_value
        (type* properties, const key_type& key, const point& t)
      {
        properties->set (key + ".x", to_property (t._x));
        properties->set (key + ".y", to_property (t._y));
      }

      template<> point retrieve_value
        (const type& properties, const key_type& key)
      {
        return point ( from_property<int> (properties.get_val (key + ".x"))
                     , from_property<int> (properties.get_val (key + ".y"))
                     );
      }
    }
  }
}

BOOST_AUTO_TEST_CASE (store_struct_list)
{
  typedef std::list<point> list_type;

  point values[] = {point (0, 0), point (-1, 1), point (-2, 2), point (-3, 3)};
  list_type orig (values, values + sizeof (values)/sizeof (*values));

  std::string where ("struct_list");

  p.set_container (where, orig);

  BOOST_REQUIRE_EQUAL (orig, p.get_container<list_type> (where));
}

namespace we
{
  namespace type
  {
    namespace property
    {
      template<> void store_value
        (type* properties, const key_type& key, const std::pair<const int,int>& t)
      {
        properties->set (key + ".first", to_property (t.first));
        properties->set (key + ".second", to_property (t.second));
      }

      template<> std::pair<const int,int> retrieve_value
        (const type& properties, const key_type& key)
      {
        return std::make_pair
          ( from_property<int> (properties.get_val (key + ".first"))
          , from_property<int> (properties.get_val (key + ".second"))
          );
      }
    }
  }
}

typedef std::map<int,int> map_type;
BOOST_TEST_DONT_PRINT_LOG_VALUE (map_type);

BOOST_AUTO_TEST_CASE (store_map)
{
  map_type orig;
  orig[1] = -1;
  orig[2] = -2;

  std::string where ("map");

  p.set_container (where, orig);

  BOOST_REQUIRE_EQUAL (orig, p.get_container<map_type> (where));
}
