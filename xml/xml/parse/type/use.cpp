// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/use.hpp>

#include <xml/parse/id/mapper.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      use_type::use_type ( ID_CONS_PARAM(use)
                         , PARENT_CONS_PARAM(transition)
                         , const std::string& name
                         )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _name (name)
      {
        _id_mapper->put (_id, *this);
      }

      const std::string& use_type::name() const
      {
        return _name;
      }

      id::ref::use use_type::clone
                  (const boost::optional<parent_id_type>& parent) const
      {
        return use_type
          ( id_mapper()->next_id()
          , id_mapper()
          , parent
          , _name
          ).make_reference_id();
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, const use_type& u)
        {
          s.open ("use");
          s.attr ("name", u.name());
          s.close ();
        }
      }
    }
  }
}
