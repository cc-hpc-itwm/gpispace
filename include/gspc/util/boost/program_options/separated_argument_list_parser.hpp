#pragma once

#include <boost/program_options/option.hpp>

#include <map>
#include <string>
#include <utility>
#include <vector>




      namespace gspc::util::boost::program_options
      {
        struct separated_argument_list_parser
        {
          std::map< std::string              // open
                  , std::pair< std::string   // close
                            , std::string   // option
                            >
                  > _sections;

          separated_argument_list_parser (decltype (_sections));
          separated_argument_list_parser ( std::string open
                                        , std::string close
                                        , std::string option
                                        );

          std::vector<::boost::program_options::option>
            operator() (std::vector<std::string>& args) const;
        };
      }
