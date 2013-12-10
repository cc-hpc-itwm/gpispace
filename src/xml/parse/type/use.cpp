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
                         , const util::position_type& pod
                         , const std::string& name
                         )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
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
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return use_type
          ( new_id
          , new_mapper
          , parent
          , _position_of_definition
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
