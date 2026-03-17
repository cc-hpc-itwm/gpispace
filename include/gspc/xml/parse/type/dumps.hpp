// Copyright (C) 2012-2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//! \note To prevent including nearly everything, it is required to
//! include dumps.hpp AFTER including types used when calling dumps().

#include <gspc/util/xml.hpp>




      namespace gspc::xml::parse::type::dump
      {
        template<typename IT>
        void dumps ( ::gspc::util::xml::xmlstream & s
                   , IT pos
                   , IT const& end
                   )
        {
          for (; pos != end; ++pos)
          {
            ::gspc::xml::parse::type::dump::dump (s, *pos);
          }
        }

        template<typename IT, typename T>
        void dumps ( ::gspc::util::xml::xmlstream & s
                   , IT pos
                   , IT const& end
                   , T const& x
                   )
        {
          for (; pos != end; ++pos)
          {
            ::gspc::xml::parse::type::dump::dump (s, *pos, x);
          }
        }

        template<typename Container>
        void dumps ( ::gspc::util::xml::xmlstream& s
                   , Container container
                   )
        {
          for (auto const& val : container)
          {
            ::gspc::xml::parse::type::dump::dump (s, val);
          }
        }

        template<typename Container, typename T>
        void dumps ( ::gspc::util::xml::xmlstream& s
                   , Container container
                   , T const& x
                   )
        {
          for (auto val : container)
          {
            ::gspc::xml::parse::type::dump::dump (s, val, x);
          }
        }
      }
