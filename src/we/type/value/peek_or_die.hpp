#pragma once

#include <we/type/value/from_value.hpp>
#include <we/type/value/path/join.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/show.hpp>

#include <boost/format.hpp>
#include <boost/optional.hpp>

#include <stdexcept>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      template<typename T>
        T peek_or_die ( value_type const& store
                      , std::list<std::string> const& path
                      )
      {
        boost::optional<value_type const&> const value (peek (path, store));

        if (!value)
        {
          throw std::logic_error
            (( boost::format ("Could not peek '%1%' from '%2%'")
             % path::join (path)
             % show (store)
             ).str()
            );
        }

        return from_value<T> (value.get());
      }
    }
  }
}
