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
