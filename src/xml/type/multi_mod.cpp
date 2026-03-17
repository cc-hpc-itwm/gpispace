// Copyright (C) 2019-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/multi_mod.hpp>

#include <gspc/xml/parse/error.hpp>
#include <gspc/xml/parse/type/function.hpp>
#include <gspc/xml/parse/util/valid_name.hpp>

#include <gspc/util/xml.hpp>

#include <gspc/assert.hpp>
#include <optional>



    namespace gspc::xml::parse::type
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

      std::optional<::gspc::we::type::eureka_id_type> const&
        multi_module_type::eureka_id() const
      {
        return _modules.begin()->second.eureka_id();
      }

      namespace dump
      {
        void dump ( ::gspc::util::xml::xmlstream& s
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
