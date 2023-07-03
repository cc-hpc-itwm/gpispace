// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <bin/run.hpp>
#include <util/print_exception.hpp>

#include <we/type/value/read.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace
{
  namespace option_name
  {
    constexpr char const* const implementation ("implementation");
    constexpr char const* const user_data ("user-data");
  }

  struct existing_path : public ::boost::filesystem::path
  {
    existing_path (std::string const& option)
      : ::boost::filesystem::path (::boost::filesystem::canonical (option))
    {
      if (!::boost::filesystem::exists (*this))
      {
        throw ::boost::program_options::invalid_option_value
          ((::boost::format ("%1% does not exist.") % *this).str());
      }
    }

    existing_path (::boost::filesystem::path const& path)
      : existing_path (path.string())
    {}
  };

  void validate ( ::boost::any& v
                , std::vector<std::string> const& values
                , existing_path*
                , int
                )
  {
    ::boost::program_options::validators::check_first_occurrence (v);
    auto const value
      (::boost::program_options::validators::get_single_string (values));
    v = ::boost::any (existing_path (value));
  }
}

int main (int argc, char** argv)
try
{
  stochastic_with_heureka::workflow_result const workflow_result
    ( stochastic_with_heureka::run
        ( argc
        , argv
        , []
          {
            ::boost::program_options::options_description options;
            options.add_options()
              ( option_name::implementation
              , ::boost::program_options::value<existing_path>()->required()
              , "shared object for implementation"
              )
              ( option_name::user_data
              , ::boost::program_options::value<std::string>()->required()
              , "user data to be passed into implementation"
              );
            return options;
          }()
        , [] ( ::boost::program_options::variables_map const& vm
             , ::boost::filesystem::path
             )
          {
            return vm[option_name::implementation].as<existing_path>();
          }
        , [] (::boost::program_options::variables_map const& vm)
          {
            return ::boost::get<we::type::bytearray>
              ( pnet::type::value::read
                  (vm[option_name::user_data].as<std::string>())
              );
          }
        )
    );

  std::cout << "got_heureka = "
            << std::boolalpha << workflow_result.got_heureka() << std::endl;
  std::cout << "number_of_rolls_done = "
            << workflow_result.number_of_rolls_done() << std::endl;
  std::cout << "result = " << workflow_result.result() << std::endl;

  return 0;
}
catch (std::exception const& e)
{
  std::cout << "EXCEPTION: " << util::print_exception (e) << std::endl;

  return -1;
}
