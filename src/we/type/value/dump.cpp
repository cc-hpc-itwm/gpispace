// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/value/dump.hpp>
#include <we/type/value/show.hpp>

#include <fhg/util/xml.hpp>

#include <FMT/we/type/value/show.hpp>
#include <fmt/core.h>
#include <sstream>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        class visitor_dump
          : public ::boost::static_visitor<fhg::util::xml::xmlstream&>
        {
        public:
          visitor_dump ( fhg::util::xml::xmlstream& os
                       , unsigned int depth
                       , ::boost::optional<std::string const&> parent = ::boost::none
                       )
            : _os (os)
            , _depth (depth)
            , _parent (parent)
          {}

          fhg::util::xml::xmlstream& operator() (structured_type const& m) const
          {
            if (_parent)
            {
              _os.open ("properties");
              _os.attr ("name", *_parent);
            }

            for (std::pair<std::string, value_type> const& kv : m)
            {
              ::boost::apply_visitor
                (visitor_dump (_os, _depth + 1, kv.first), kv.second);
            }

            if (_parent)
            {
              _os.close();
            }

            return _os;
          }
          template<typename T>
          fhg::util::xml::xmlstream& operator() (T const& value) const
          {
            if (!_parent)
            {
              throw std::runtime_error
                { fmt::format ("cannot dump the plain value '{}'", show (value))
                };
            }
            if (_depth < 2)
            {
              throw std::runtime_error
                { fmt::format ( "cannot dump the single level property"
                                " with key '{}' and value '{}'"
                              , *_parent
                              , show (value)
                              )
                };
            }

            _os.open ("property");
            _os.attr ("key", *_parent);

            std::ostringstream oss;
            oss << show (value);
            _os.content (oss.str());

            _os.close();

            return _os;
          }

        private:
          fhg::util::xml::xmlstream& _os;
          unsigned int _depth;
          ::boost::optional<std::string const&> _parent;
        };
      }

      fhg::util::xml::xmlstream& dump ( fhg::util::xml::xmlstream& os
                                      , value_type const& value
                                      )
      {
        return ::boost::apply_visitor (visitor_dump (os, 0), value);
      }
    }
  }
}
