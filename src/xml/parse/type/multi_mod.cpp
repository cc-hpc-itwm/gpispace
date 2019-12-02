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
          throw std::logic_error ( "cannot include module '"
                                 + mod.name() + "' with preferences"
                                 + " without a target"
                                 );
        }

        auto const& status = _modules.emplace
                              (std::make_pair (*mod.target()
                                              , mod
                                              )
                              );

        if (!status.second)
        {
          throw std::logic_error ( "duplicate module '" + mod.name()
                                 + "' for target '" + *mod.target()
                                 + "' module "
                                 + status.first->second.name()
                                 + "' already exists"
                                 );
        }
      }

      multi_module_map const& multi_module_type::modules() const
      {
        return _modules;
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
