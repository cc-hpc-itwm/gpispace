// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _XML_PARSE_PARSER_HPP
#define _XML_PARSE_PARSER_HPP

#include <xml/parse/id/types.fwd.hpp>
#include <xml/parse/state.fwd.hpp>

#include <we/mgmt/type/activity.fwd.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    id::ref::function just_parse (state::type&, const std::string&);

    void post_processing_passes (const id::ref::function&, state::type*);

    void generate_cpp (const id::ref::function&, const state::type&);
    void dump_xml (const id::ref::function&, const state::type&);

    //! \todo Not in parser, but somewhere else?
    we::mgmt::type::activity_t xml_to_we
      ( const xml::parse::id::ref::function& function
      , const xml::parse::state::type& state
      );
  }
}

#endif
