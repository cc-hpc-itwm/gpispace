#include <pnete/ui/graph/style/store.hpp>

#include <boost/bind.hpp>

#include <iostream>

typedef std::string value_type;
typedef fhg::pnete::ui::graph::style::store::cached_predicates<std::string, value_type> store_type;
typedef store_type::optional_value_type optional_value_type;

std::ostream& operator << ( std::ostream& s
                          , const optional_value_type& v
                          )
{
  if (v)
    {
      s << "Just " << *v;
    }
  else
    {
      s << "Nothing";
    }

  return s;
}

static std::size_t count_eval;

optional_value_type beep_if ( const std::string& what
                            , const std::string& key
                            )
{
  static const std::string b ("beep");

  std::cerr << "EVAL: beep_if (" << what << ", " << key << ")" << std::endl;
  ++count_eval;

  if (what == key)
    {
      return optional_value_type (boost::ref (b));
    }

  return boost::none;
}



optional_value_type short_id ( const std::size_t& len
                             , const std::string& key
                             )
{
  static std::string s;

  std::cerr << "EVAL: short_id (" << len << ", " << key << ")" << std::endl;
  ++count_eval;

  if (key.size() < len)
    {
      s = key;

      return optional_value_type (boost::ref (s));
    }

  return boost::none;
}

optional_value_type fallback (const std::string&)
{
  static const std::string f ("fallback");

  return optional_value_type (boost::ref (f));
}

#define REQUIRE(b) \
  if (!(b)) { std::cerr << "FAILURE in line " << __LINE__ << std::endl; \
              exit (EXIT_FAILURE); \
            }

optional_value_type get ( const store_type& store
                        , const std::string& key
                        )
{
  count_eval = 0;
  const optional_value_type r (store.get (key));
  std::cout << r << std::endl;
  return r;
}

int main ()
{
  store_type store;

  store.push (boost::bind (&beep_if, std::string ("what"), _1));
  store.push (boost::bind (&short_id, 4, _1));

  {
    const optional_value_type res (get (store, "what"));
    REQUIRE (count_eval == 1);
    REQUIRE (res);
    REQUIRE (*res == "beep");
  }
  {
    const optional_value_type res (get (store, "id"));
    REQUIRE (count_eval == 2);
    REQUIRE (res);
    REQUIRE (*res == "id");
  }
  {
    const optional_value_type res (get (store, "idd"));
    REQUIRE (count_eval == 2);
    REQUIRE (res);
    REQUIRE (*res == "idd");
  }
  {
    const optional_value_type res (get (store, "to long"));
    REQUIRE (count_eval == 2);
    REQUIRE (!res);
  }

  {
    const optional_value_type res (get (store, "what"));
    REQUIRE (count_eval == 0);
    REQUIRE (res);
    REQUIRE (*res == "beep");
  }
  {
    const optional_value_type res (get (store, "id"));
    REQUIRE (count_eval == 0);
    REQUIRE (res);
    REQUIRE (*res == "idd"); // yes, because its the same ref but no re-eval!
  }
  {
    const optional_value_type res (get (store, "idd"));
    REQUIRE (count_eval == 0);
    REQUIRE (res);
    REQUIRE (*res == "idd");
  }
  {
    const optional_value_type res (get (store, "to long"));
    REQUIRE (count_eval == 0);
    REQUIRE (!res);
  }

  store.push (&fallback);

  {
    const optional_value_type res (get (store, "what"));
    REQUIRE (count_eval == 1);
    REQUIRE (res);
    REQUIRE (*res == "beep");
  }
  {
    const optional_value_type res (get (store, "id"));
    REQUIRE (count_eval == 2);
    REQUIRE (res);
    REQUIRE (*res == "id");
  }
  {
    const optional_value_type res (get (store, "idd"));
    REQUIRE (count_eval == 2);
    REQUIRE (res);
    REQUIRE (*res == "idd");
  }
  {
    const optional_value_type res (get (store, "to long"));
    REQUIRE (count_eval == 2);
    REQUIRE (res);
    REQUIRE (*res == "fallback");
  }

  {
    const optional_value_type res (get (store, "what"));
    REQUIRE (count_eval == 0);
    REQUIRE (res);
    REQUIRE (*res == "beep");
  }
  {
    const optional_value_type res (get (store, "id"));
    REQUIRE (count_eval == 0);
    REQUIRE (res);
    REQUIRE (*res == "idd"); // yes, because its the same ref but no re-eval!
  }
  {
    const optional_value_type res (get (store, "idd"));
    REQUIRE (count_eval == 0);
    REQUIRE (res);
    REQUIRE (*res == "idd");
  }
  {
    const optional_value_type res (get (store, "to long"));
    REQUIRE (count_eval == 0);
    REQUIRE (res);
    REQUIRE (*res == "fallback");
  }

  return EXIT_SUCCESS;
}
