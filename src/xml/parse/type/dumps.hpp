// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//! \note To prevent including nearly everything, it is required to
//! include dumps.hpp AFTER including types used when calling dumps().

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      namespace dump
      {
        template<typename IT>
        void dumps ( ::fhg::util::xml::xmlstream & s
                   , IT pos
                   , IT const& end
                   )
        {
          for (; pos != end; ++pos)
          {
            ::xml::parse::type::dump::dump (s, *pos);
          }
        }

        template<typename IT, typename T>
        void dumps ( ::fhg::util::xml::xmlstream & s
                   , IT pos
                   , IT const& end
                   , T const& x
                   )
        {
          for (; pos != end; ++pos)
          {
            ::xml::parse::type::dump::dump (s, *pos, x);
          }
        }

        template<typename Container>
        void dumps ( ::fhg::util::xml::xmlstream& s
                   , Container container
                   )
        {
          for (typename Container::value_type const& val : container)
          {
            ::xml::parse::type::dump::dump (s, val);
          }
        }

        template<typename Container, typename T>
        void dumps ( ::fhg::util::xml::xmlstream& s
                   , Container container
                   , T const& x
                   )
        {
          for (auto val : container)
          {
            ::xml::parse::type::dump::dump (s, val, x);
          }
        }
      }
    }
  }
}
