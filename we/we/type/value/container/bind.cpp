// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/container/bind.hpp>

#include <we/type/value/put.hpp>

#include <we/type/value/mk_structured.hpp>

namespace value
{
  namespace container
  {
    void bind ( type & container
              , const std::string & key
              , const std::list<std::string>& path
              , const value::type & value
              )
    {
      value::put ( path
                 , value::mk_structured_or_keep (container[key])
                 , value
                 );
    }
    void bind ( type & container
              , const std::string & key
              , const std::string& path
              , const value::type & value
              )
    {
      value::put ( path
                 , value::mk_structured_or_keep (container[key])
                 , value
                 );
    }
    void bind ( type & container
              , const key_vec_t & key_vec
              , const value::type & value
              )
    {
      if (key_vec.size() == 0)
        {
          throw std::runtime_error ("value::container::bind []");
        }

      key_vec_t::const_iterator pos (key_vec.begin());
      const std::string& key (*pos); ++pos;

      value::put ( pos
                 , key_vec.end()
                 , value::mk_structured_or_keep (container[key])
                 , value
                 );
    }
  }
}
