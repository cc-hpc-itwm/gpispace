#include <pnete/ui/graph/style/param.hpp>

#include <boost/bind.hpp>

#include <iostream>

namespace p = fhg::pnete::ui::graph::style::param;

typedef std::string value_type;

std::ostream& operator << ( std::ostream& s
                          , const boost::optional<value_type>& v
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

boost::optional<value_type> beep_if ( const std::string& what
                                    , const std::string& key
                                    )
{
  std::cerr << "EVAL: beep_if (" << what << ", " << key << ")" << std::endl;

  if (what == key)
    {
      return std::string ("beep");
    }

  return boost::none;
}

boost::optional<value_type> short_id ( const std::size_t& len
                                     , const std::string& key
                                     )
{
  std::cerr << "EVAL: short_id (" << len << ", " << key << ")" << std::endl;

  if (key.size() < len)
    {
      return key;
    }

  return boost::none;
}

boost::optional<value_type> fallback (const std::string&)
{
  return std::string ("default");
}

int main ()
{
  p::store<std::string, value_type> store;

  store.push (boost::bind (&beep_if, std::string ("what"), _1));
  store.push (boost::bind (&short_id, 3, _1));

  std::cout << store.get ("what") << std::endl;
  std::cout << store.get ("id") << std::endl;
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
