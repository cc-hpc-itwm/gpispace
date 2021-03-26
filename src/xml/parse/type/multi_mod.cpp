// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <xml/parse/type/multi_mod.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/util/valid_name.hpp>

#include <fhg/util/xml.hpp>

#include <fhg/assert.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      void multi_module_type::add (module_type const& mod)
      {
        if (!mod.target())
        {
          throw error::missing_target_for_module
            ( mod.name()
            , mod.position_of_definition()
            );
        }

        auto const& status = _modules.emplace (*mod.target(), mod);

        if (!status.second)
        {
          throw error::duplicate_module_for_target
            ( mod.name()
            , *mod.target()
            , mod.position_of_definition()
            );
        }
      }

      multi_module_map const& multi_module_type::modules() const
      {
        return _modules;
      }

      boost::optional<we::type::eureka_id_type> const&
        multi_module_type::eureka_id() const
      {
        return _modules.begin()->second.eureka_id();
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream& s
                  , const multi_module_type& m_map
                  )
        {
          for (auto const& m_loc : m_map.modules())
          {
            const module_type& m = m_loc.second;

            s.open ("module");
            s.attr ("name", m.name());
            s.attr ("function", dump_fun (m));
            s.attr ("target", m.target());

            for (const std::string& inc : m.cincludes())
            {
              s.open ("cinclude");
              s.attr ("href", inc);
              s.close ();
            }

            for (const std::string& flag : m.ldflags())
            {
              s.open ("ld");
              s.attr ("flag", flag);
              s.close ();
            }

            for (const std::string& flag : m.cxxflags())
            {
              s.open ("cxx");
              s.attr ("flag", flag);
              s.close ();
            }

            if (m.code())
            {
              s.open ("code");
              s.content ("<![CDATA[" + *m.code() + "]]>");
              s.close ();
            }

            s.close ();
          }
        }
      }
    }
  }
}
