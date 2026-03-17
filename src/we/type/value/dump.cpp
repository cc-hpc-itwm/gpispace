// Copyright (C) 2013-2014,2020-2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/value/dump.hpp>
#include <gspc/we/type/value/show.hpp>

#include <gspc/util/xml.hpp>

#include <gspc/we/type/value/show.formatter.hpp>
#include <fmt/core.h>
#include <sstream>



    namespace gspc::pnet::type::value
    {
      namespace
      {
        class visitor_dump
          : public ::boost::static_visitor<util::xml::xmlstream&>
        {
        public:
          visitor_dump ( util::xml::xmlstream& os
                       , unsigned int depth
                       , std::optional<std::reference_wrapper<std::string const>> parent = std::nullopt
                       )
            : _os (os)
            , _depth (depth)
            , _parent (parent)
          {}

          util::xml::xmlstream& operator() (structured_type const& m) const
          {
            if (_parent)
            {
              _os.open ("properties");
              _os.attr ("name", _parent->get());
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
          util::xml::xmlstream& operator() (T const& value) const
          {
            if (!_parent)
            {
              throw std::runtime_error
                { fmt::format ("cannot dump the plain value '{}'", show (value_type (value)))
                };
            }
            if (_depth < 2)
            {
              throw std::runtime_error
                { fmt::format ( "cannot dump the single level property"
                                " with key '{}' and value '{}'"
                              , _parent->get()
                              , show (value_type (value))
                              )
                };
            }

            _os.open ("property");
            _os.attr ("key", _parent->get());

            std::ostringstream oss;
            oss << show (value_type (value));
            _os.content (oss.str());

            _os.close();

            return _os;
          }

        private:
          util::xml::xmlstream& _os;
          unsigned int _depth;
          std::optional<std::reference_wrapper<std::string const>> _parent;
        };
      }

      util::xml::xmlstream& dump ( util::xml::xmlstream& os
                                      , value_type const& value
                                      )
      {
        return ::boost::apply_visitor (visitor_dump (os, 0), value);
      }
    }
