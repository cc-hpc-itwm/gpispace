// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/bytearray.hpp>
#include <we/type/value.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <functional>
#include <map>
#include <string>

namespace stochastic_with_heureka
{
  struct workflow_result
  {
    workflow_result
      (std::multimap<std::string, pnet::type::value::value_type> const&);

    we::type::bytearray const& result() const
    {
      return _result;
    }
    bool got_heureka() const
    {
      return _got_heureka;
    }
    unsigned long number_of_rolls_done() const
    {
      return _number_of_rolls_done;
    }

  private:
    we::type::bytearray _result;
    bool _got_heureka;
    unsigned long _number_of_rolls_done;
  };

  workflow_result run
    ( int argc
    , char** argv
    , ::boost::optional<::boost::program_options::options_description>
    , std::function< ::boost::filesystem::path
                       ( ::boost::program_options::variables_map const&
                       , ::boost::filesystem::path installation_dir
                       )
                   > implementation
    , std::function< we::type::bytearray
                     (::boost::program_options::variables_map const&)
                   >
    );

  workflow_result run
    ( int argc
    , char** argv
    , ::boost::optional<::boost::program_options::options_description>
    , std::string const implementation
    , std::function< we::type::bytearray
                     (::boost::program_options::variables_map const&)
                   >
    );
}
