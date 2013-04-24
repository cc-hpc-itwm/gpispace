// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_signature
#include <boost/test/unit_test.hpp>

#include <we/type/signature.hpp>
#include <we/type/signature/show.hpp>

#include <sstream>

BOOST_AUTO_TEST_CASE (signature_show)
{
  using pnet::type::signature::signature_type;
  using pnet::type::signature::structured_type;
  using pnet::type::signature::field_type;
  using pnet::type::signature::show;

#define CHECK(_expected,_sig...)                \
  {                                             \
    std::ostringstream oss;                     \
                                                \
    oss << show (signature_type (_sig));        \
                                                \
    BOOST_CHECK_EQUAL (oss.str(), _expected);   \
  }

  CHECK ("literal", std::string ("literal"));

  {
    std::list<signature_type> l;
    CHECK ("list{}", l);

    l.push_back (std::string ("a"));
    CHECK ("list{a}", l);

    l.push_back (std::string ("b"));
    CHECK ("list{a|b}", l);

    std::set<signature_type> s;
    CHECK ("set{}", s);

    s.insert (std::string ("b"));
    CHECK ("set{b}", s);

    s.insert (std::string ("a"));
    CHECK ("set{a|b}", s);

    s.insert (l);
    CHECK ("set{a|b|list{a|b}}", s);

    std::map<signature_type, signature_type> m;
    CHECK ("map{}", m);

    m[std::string("a")] = std::string("b");
    CHECK ("map{a->b}", m);

    m[l] = s;
    CHECK ("map{a->b|list{a|b}->set{a|b|list{a|b}}}", m);
  }

  {
    structured_type m;
    m[std::string ("a")] = std::string ("b");

    field_type f ("f", m);

    CHECK ("f => [a :: b]", f);
  }

#undef CHECK
}
