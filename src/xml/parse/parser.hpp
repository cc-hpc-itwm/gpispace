// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/function.hpp>

#include <we/type/transition.fwd.hpp>

#include <boost/filesystem.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    type::function_type just_parse (state::type&, std::istream&);
    type::function_type just_parse
      (state::type&, const boost::filesystem::path&);

    void post_processing_passes (type::function_type&, state::type*);

    void generate_cpp (const type::function_type&, const state::type&);
    void dump_xml (const type::function_type&, const state::type&);

    //! \todo Not in parser, but somewhere else?
    we::type::transition_t xml_to_we
      ( const xml::parse::type::function_type& function
      , const xml::parse::state::type& state
      );
  }
}
