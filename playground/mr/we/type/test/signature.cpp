// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_signature
#include <boost/test/unit_test.hpp>

#include <we/type/signature.hpp>
#include <we/type/signature/show.hpp>
#include <we/type/signature/cpp.hpp>
#include <we/type/signature/dump.hpp>
#include <we/type/signature/name.hpp>
#include <we/type/signature/signature.hpp>

#include <sstream>

BOOST_AUTO_TEST_CASE (signature_show)
{
#define CHECK(_expected,_sig...)                \
  {                                             \
    using pnet::type::signature::show;          \
                                                \
    std::ostringstream oss;                     \
                                                \
    oss << show (signature_type (_sig));        \
                                                \
    BOOST_CHECK_EQUAL (oss.str(), _expected);   \
  }

  using pnet::type::signature::signature_type;
  using pnet::type::signature::structured_type;
  using pnet::type::signature::structure_type;
  using pnet::type::signature::field_type;

  CHECK ("float", std::string ("float"));

  structure_type f;

  CHECK ("s :: []", structured_type (std::make_pair ("s", f)));

  f.push_back (std::make_pair (std::string ("x"), std::string ("float")));

  CHECK ("s :: [x :: float]", structured_type (std::make_pair ("s", f)));

  f.push_back (std::make_pair (std::string ("y"), std::string ("float")));

  CHECK ( "point2D :: [x :: float, y :: float]"
        , structured_type (std::make_pair ("point2D", f))
        );

  structure_type ps;

  ps.push_back (std::make_pair (std::string ("p"), std::string ("point2D")));
  ps.push_back (structured_type (std::make_pair ("q", f)));

  CHECK ( "line2D :: [p :: point2D, q :: [x :: float, y :: float]]"
        , structured_type (std::make_pair ("line2D", ps))
        );
#undef CHECK
}

BOOST_AUTO_TEST_CASE (signature_dump)
{
#define CHECK(_expected,_sig...)                        \
  {                                                     \
    using pnet::type::signature::dump;                  \
                                                        \
    std::ostringstream oss;                             \
                                                        \
    oss << dump (structured_type (_sig));               \
                                                        \
    BOOST_CHECK_EQUAL (oss.str(), _expected);           \
  }

  using pnet::type::signature::structured_type;
  using pnet::type::signature::structure_type;
  using pnet::type::signature::field_type;

  structure_type f;

  CHECK ( "<struct name=\"s\"/>\n"
        , structured_type (std::make_pair ("s", f))
        );

  f.push_back (std::make_pair (std::string ("x"), std::string ("float")));

  CHECK ( "<struct name=\"s\">\n"
          "  <field name=\"x\" type=\"float\"/>\n"
          "</struct>\n"
        , structured_type (std::make_pair ("s", f))
        );

  f.push_back (std::make_pair (std::string ("y"), std::string ("float")));

  CHECK ( "<struct name=\"point2D\">\n"
          "  <field name=\"x\" type=\"float\"/>\n"
          "  <field name=\"y\" type=\"float\"/>\n"
          "</struct>\n"
        , structured_type (std::make_pair ("point2D", f))
        );

  structure_type ps;

  ps.push_back (std::make_pair (std::string ("p"), std::string ("point2D")));
  ps.push_back (structured_type (std::make_pair ("q", f)));

  CHECK ( "<struct name=\"line2D\">\n"
          "  <field name=\"p\" type=\"point2D\"/>\n"
          "  <struct name=\"q\">\n"
          "    <field name=\"x\" type=\"float\"/>\n"
          "    <field name=\"y\" type=\"float\"/>\n"
          "  </struct>\n"
          "</struct>\n"
        , structured_type (std::make_pair ("line2D", ps))
        );

#undef CHECK
}

#include <fstream>

BOOST_AUTO_TEST_CASE (signature_cpp)
{
#define CHECK_HEADER(_expected,_sig...)                 \
  {                                                     \
    using pnet::type::signature::cpp::header;           \
                                                        \
    std::ostringstream oss;                             \
                                                        \
    oss << header (structured_type (_sig));             \
                                                        \
    BOOST_CHECK_EQUAL (oss.str(), _expected);           \
                                                        \
  }
#define CHECK_IMPL(_expected,_sig...)                   \
  {                                                     \
    using pnet::type::signature::cpp::impl;             \
                                                        \
    std::ostringstream oss;                             \
                                                        \
    oss << impl (structured_type (_sig));               \
                                                        \
    BOOST_CHECK_EQUAL (oss.str(), _expected);           \
    {std::ofstream f ("/u/r/rahn/out.oss"); f << oss.str();}    \
    {std::ofstream f ("/u/r/rahn/out.exp"); f << _expected;}    \
                                                        \
  }

  using pnet::type::signature::structured_type;
  using pnet::type::signature::structure_type;
  using pnet::type::signature::field_type;

  structure_type f;
  f.push_back (std::make_pair (std::string ("x"), std::string ("float")));
  f.push_back (std::make_pair (std::string ("y"), std::string ("float")));

  CHECK_HEADER
    ("#include <we/type/value.hpp>\n"
     "#include <boost/serialization/nvp.hpp>\n"
     "\n"
     "namespace point2D\n"
     "{\n"
     "  struct type\n"
     "  {\n"
     "    float x;\n"
     "    float y;\n"
     "\n"
     "    type();\n"
     "    explicit type (const pnet::type::value::value_type&);\n"
     "\n"
     "    template<typename Archive>\n"
     "    void serialize (Archive& ar, const unsigned int)\n"
     "    {\n"
     "      ar & BOOST_SERIALIZATION_NVP (x);\n"
     "      ar & BOOST_SERIALIZATION_NVP (y);\n"
     "    }\n"
     "  };\n"
     "\n"
     "  pnet::type::value::value_type value (const type&);\n"
     "}"
     , std::make_pair ("point2D", f)
    );

  CHECK_IMPL
    ("#include <we/type/value/poke.hpp>\n"
     "#include <we/field.hpp>\n"
     "#include <we/signature_of.hpp>\n"
     "\n"
     "namespace point2D\n"
     "{\n"
     "  type::type()\n"
     "    : x()\n"
     "    , y()\n"
     "  {}\n"
     "  type::type (const pnet::type::value::value_type& v)\n"
     "    : x (pnet::field_as<float> (pnet::path (\"x\"), v, std::string(\"float\")))\n"
     "    , y (pnet::field_as<float> (pnet::path (\"y\"), v, std::string(\"float\")))\n"
     "  {}\n"
     "  pnet::type::value::value_type value (const type& x)\n"
     "  {\n"
     "    pnet::type::value::value_type v;\n"
     "    pnet::type::value::poke (\"x\", v, x.x);\n"
     "    pnet::type::value::poke (\"y\", v, x.y);\n"
     "    return v;\n"
     "  }\n"
     "}"
     , std::make_pair ("point2D", f)
    );

  {
    structure_type a;
    a.push_back (std::make_pair (std::string ("i"), std::string ("int")));
    structure_type b;
    b.push_back (structured_type (std::make_pair ("a", a)));
    structure_type c;
    c.push_back (structured_type (std::make_pair ("b", b)));

    CHECK_HEADER
      ("#include <we/type/value.hpp>\n"
       "#include <boost/serialization/nvp.hpp>\n"
       "\n"
       "namespace c\n"
       "{\n"
       "  namespace b\n"
       "  {\n"
       "    namespace a\n"
       "    {\n"
       "      struct type\n"
       "      {\n"
       "        int i;\n"
       "\n"
       "        type();\n"
       "        explicit type (const pnet::type::value::value_type&);\n"
       "\n"
       "        template<typename Archive>\n"
       "        void serialize (Archive& ar, const unsigned int)\n"
       "        {\n"
       "          ar & BOOST_SERIALIZATION_NVP (i);\n"
       "        }\n"
       "      };\n"
       "\n"
       "      pnet::type::value::value_type value (const type&);\n"
       "    }\n"
       "    struct type\n"
       "    {\n"
       "      a::type a;\n"
       "\n"
       "      type();\n"
       "      explicit type (const pnet::type::value::value_type&);\n"
       "\n"
       "      template<typename Archive>\n"
       "      void serialize (Archive& ar, const unsigned int)\n"
       "      {\n"
       "        ar & BOOST_SERIALIZATION_NVP (a);\n"
       "      }\n"
       "    };\n"
       "\n"
       "    pnet::type::value::value_type value (const type&);\n"
       "  }\n"
       "  struct type\n"
       "  {\n"
       "    b::type b;\n"
       "\n"
       "    type();\n"
       "    explicit type (const pnet::type::value::value_type&);\n"
       "\n"
       "    template<typename Archive>\n"
       "    void serialize (Archive& ar, const unsigned int)\n"
       "    {\n"
       "      ar & BOOST_SERIALIZATION_NVP (b);\n"
       "    }\n"
       "  };\n"
       "\n"
       "  pnet::type::value::value_type value (const type&);\n"
       "}"
      , std::make_pair ("c", c)
      );

    CHECK_IMPL
      ("#include <we/type/value/poke.hpp>\n"
       "#include <we/field.hpp>\n"
       "#include <we/signature_of.hpp>\n"
       "\n"
       "namespace c\n"
       "{\n"
       "  namespace b\n"
       "  {\n"
       "    namespace a\n"
       "    {\n"
       "      type::type()\n"
       "        : i()\n"
       "      {}\n"
       "      type::type (const pnet::type::value::value_type& v)\n"
       "        : i (pnet::field_as<int> (pnet::path (\"i\"), v, std::string(\"int\")))\n"
       "      {}\n"
       "      pnet::type::value::value_type value (const type& x)\n"
       "      {\n"
       "        pnet::type::value::value_type v;\n"
       "        pnet::type::value::poke (\"i\", v, x.i);\n"
       "        return v;\n"
       "      }\n"
       "    }\n"
       "    type::type()\n"
       "      : a()\n"
       "    {}\n"
       "    type::type (const pnet::type::value::value_type& v)\n"
       "      : a (pnet::field (pnet::path (\"a\"), v, pnet::signature_of (a::value (a::type()))))\n"
       "    {}\n"
       "    pnet::type::value::value_type value (const type& x)\n"
       "    {\n"
       "      pnet::type::value::value_type v;\n"
       "      pnet::type::value::poke (\"a\", v, a::value (x.a));\n"
       "      return v;\n"
       "    }\n"
       "  }\n"
       "  type::type()\n"
       "    : b()\n"
       "  {}\n"
       "  type::type (const pnet::type::value::value_type& v)\n"
       "    : b (pnet::field (pnet::path (\"b\"), v, pnet::signature_of (b::value (b::type()))))\n"
       "  {}\n"
       "  pnet::type::value::value_type value (const type& x)\n"
       "  {\n"
       "    pnet::type::value::value_type v;\n"
       "    pnet::type::value::poke (\"b\", v, b::value (x.b));\n"
       "    return v;\n"
       "  }\n"
       "}"
      , std::make_pair ("c", c)
      );
  }
#undef CHECK
}

BOOST_AUTO_TEST_CASE (name_signature)
{
  using pnet::type::signature::signature_type;
  using pnet::type::signature::signature;
  using pnet::type::signature::name;
  using pnet::type::signature::structured_type;
  using pnet::type::signature::structure_type;
  using pnet::type::signature::field_type;

  const field_type
    f (std::make_pair (std::string ("name"), std::string ("type")));

  BOOST_CHECK_EQUAL (std::string ("name"), name (f));
  BOOST_CHECK (signature_type (std::string ("type")) == signature (f));

  structure_type s;
  s.push_back (f);
  structured_type ss (structured_type (std::make_pair ("s", s)));
  const field_type fs (ss);

  BOOST_CHECK_EQUAL (std::string ("s"), name (fs));
  BOOST_CHECK (signature_type (ss) == signature (fs));
}
