// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/function.hpp>

#include <we/type/Transition.fwd.hpp>

#include <boost/filesystem.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    type::function_type just_parse (state::type&, std::istream&);
    type::function_type just_parse
      (state::type&, ::boost::filesystem::path const&);

    void post_processing_passes (type::function_type&, state::type*);

    void generate_cpp (type::function_type const&, state::type const&);
    void dump_xml (type::function_type const&, state::type const&);

    //! \todo Not in parser, but somewhere else?
    we::type::Transition xml_to_we
      ( xml::parse::type::function_type const& function
      , xml::parse::state::type const& state
      );
  }
}
