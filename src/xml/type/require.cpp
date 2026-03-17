// Copyright (C) 2012-2014,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/require.hpp>

#include <gspc/util/xml.hpp>



    namespace gspc::xml::parse::type
    {
      void requirements_type::set (require_key_type const& key)
      {
        _set.insert (key);
      }

      requirements_type::const_iterator requirements_type::begin () const
      {
        return _set.begin();
      }
      requirements_type::const_iterator requirements_type::end () const
      {
        return _set.end();
      }

      void requirements_type::join (requirements_type const& reqs)
      {
        for (auto const& req : reqs)
        {
          set (req);
        }
      }

      namespace dump
      {
        void dump ( ::gspc::util::xml::xmlstream & s
                  , requirements_type const& cs
                  )
        {
          for (auto const& cap : cs)
          {
            s.open ("require");
            s.attr ("key", cap);
            s.close ();
          }
        }
      }
    }
