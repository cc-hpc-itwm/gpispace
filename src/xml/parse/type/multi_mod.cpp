// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

      ::boost::optional<we::type::eureka_id_type> const&
        multi_module_type::eureka_id() const
      {
        return _modules.begin()->second.eureka_id();
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream& s
                  , multi_module_type const& m_map
                  )
        {
          for (auto const& m_loc : m_map.modules())
          {
            module_type const& m = m_loc.second;

            s.open ("module");
            s.attr ("name", m.name());
            s.attr ("function", dump_fun (m));
            s.attr ("target", m.target());

            for (std::string const& inc : m.cincludes())
            {
              s.open ("cinclude");
              s.attr ("href", inc);
              s.close ();
            }

            for (std::string const& flag : m.ldflags())
            {
              s.open ("ld");
              s.attr ("flag", flag);
              s.close ();
            }

            for (std::string const& flag : m.cxxflags())
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
