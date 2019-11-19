// dipti.shankar@itwm.fraunhofer.de

#include <xml/parse/type/preferences.hpp>

#include <xml/parse/error.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      preferences_type::preferences_type()
        : _list (0)
      { }


      //! \note target_name is assumed unique and is 
      //! \note insertion order reflects preference ordering
      void preferences_type::add_unique_target_in_order
        (const preference_type& target_name)
      {
        _list.push_back (target_name);
      }

      bool preferences_type::empty() const
      {
        return _list.empty();
      }

      const std::list<type::preference_type>&
        preferences_type::get_preference_list() const
      {
        return _list;
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const preferences_type & cs
                  )
        {
          s.open ("preferences");

          for (auto const& pref_type : cs.get_preference_list())
          {
            s.open ("target");
            s.content (pref_type);
            s.close ();
          }

          s.close ();
        }
      }
    }
  }
}
