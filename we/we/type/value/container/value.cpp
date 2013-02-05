// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/container/value.hpp>

#include <we/type/value/find.hpp>

namespace value
{
  namespace container
  {
    const value::type& value ( const type& container
                             , const key_vec_t& key_vec
                             )
    {
      if (key_vec.empty())
      {
        throw std::runtime_error ("value::container::value []");
      }

      key_vec_t::const_iterator key_pos (key_vec.begin());
      const std::string& key (*key_pos); ++key_pos;

      const type::const_iterator pos (container.find (key));

      if (pos == container.end())
      {
        throw exception::missing_binding (key);
      }
      else
      {
        return value::find (key_pos, key_vec.end(), pos->second);
      }
    }
  }
}
