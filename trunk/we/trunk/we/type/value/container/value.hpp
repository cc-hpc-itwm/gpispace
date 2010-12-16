// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_CONTAINER_VALUE_HPP
#define _WE_TYPE_VALUE_CONTAINER_VALUE_HPP 1

#include <we/type/value/container/type.hpp>
#include <we/type/value/container/exception.hpp>

#include <we/type/value/get.hpp>

#include <fhg/util/show.hpp>

namespace value
{
  namespace container
  {
    namespace detail
    {
      inline const value::type & find ( key_vec_t::const_iterator pos
                                      , const key_vec_t::const_iterator end
                                      , const value::type & store
                                      )
      {
        if (pos == end)
          {
            return store;
          }
        else
          {
            value::visitor::get_field get (fhg::util::show (*pos));

            return find ( pos + 1
                        , end
                        , boost::apply_visitor (get, store)
                        );
          }
      }
    }

    inline const value::type & value ( const type & container
                                     , const key_vec_t & key_vec
                                     )
    {
      switch (key_vec.size())
        {
        case 0:
          throw std::runtime_error ("value::container::value []");
        default:
          {
            const type::const_iterator pos (container.find (key_vec[0]));

            if (pos == container.end())
              {
                throw exception::missing_binding (key_vec[0]);
              }
            else
              {
                return detail::find ( key_vec.begin() + 1
                                    , key_vec.end()
                                    , pos->second
                                    );
              }
          }
        }
    }
  }
}

#endif
