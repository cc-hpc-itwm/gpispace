// Copyright (C) 2010,2012-2016,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/state.fwd.hpp>
#include <gspc/xml/parse/type/function.hpp>

#include <gspc/we/type/Transition.fwd.hpp>

#include <filesystem>

#include <string>


  namespace gspc::xml::parse
  {
    type::function_type just_parse (state::type&, std::istream&);
    type::function_type just_parse
      (state::type&, std::filesystem::path const&);

    void post_processing_passes (type::function_type&, state::type*);

    void generate_cpp (type::function_type const&, state::type const&);
    void dump_xml (type::function_type const&, state::type const&);

    //! \todo Not in parser, but somewhere else?
    ::gspc::we::type::Transition xml_to_we
      ( gspc::xml::parse::type::function_type const& function
      , gspc::xml::parse::state::type const& state
      );
  }
