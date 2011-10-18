#include <pnete/ui/graph/style/param.hpp>

#include <boost/bind.hpp>

#include <iostream>

namespace p = fhg::pnete::ui::graph::style::param;

typedef std::string value_type;
typedef fhg::pnete::ui::graph::style::param::store<std::string, value_type> store_type;
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

optional_value_type beep_if ( const std::string& what
                            , const std::string& key
                            )
{
  static const std::string b ("beep");

  std::cerr << "EVAL: beep_if (" << what << ", " << key << ")" << std::endl;

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

int main ()
{
  store_type store;

  store.push (boost::bind (&beep_if, std::string ("what"), _1));
  store.push (boost::bind (&short_id, 4, _1));

  std::cout << store.get ("what") << std::endl;
  std::cout << store.get ("id") << std::endl;
  std::cout << store.get ("idd") << std::endl;
  std::cout << store.get ("to long") << std::endl;
  std::cout << store.get ("what") << std::endl;
  std::cout << store.get ("id") << std::endl;
  std::cout << store.get ("to long") << std::endl;

  store.push (&fallback);

  std::cout << store.get ("what") << std::endl;
  std::cout << store.get ("id") << std::endl;
  std::cout << store.get ("to long") << std::endl;
  std::cout << store.get ("what") << std::endl;
  std::cout << store.get ("id") << std::endl;
  std::cout << store.get ("to long") << std::endl;

  return 0;
}
