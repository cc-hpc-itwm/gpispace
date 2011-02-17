// mirko.rahn@itwm.fraunhofer.de

#include <we/we.hpp>

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

// ************************************************************************* //

template<typename T>
static inline void dump (std::ostream & os, const T & v)
{
  for (typename T::const_iterator pos (v.begin()); pos != v.end(); ++pos)
    {
      os << "on " << pos->second << ": " << pos->first << std::endl;
    }
}

// ************************************************************************* //

namespace po = boost::program_options;

int
main (int argc, char ** argv)
{
  std::string input ("-");
  std::string output ("-");
  std::string key ("output");

  po::options_description desc("options");

  desc.add_options()
    ( "help,h", "this message")
    ( "input,i"
    , po::value<std::string>(&input)->default_value(input)
    , "input file name, - for stdin"
    )
    ( "output,o"
    , po::value<std::string>(&output)->default_value(input)
    , "output file name, - for stdout"
    )
    ( "key,k"
    , po::value<std::string>(&key)->default_value(key)
    , "key to extract: output, input, pending"
    )
    ;

  po::positional_options_description p;
  p.add("key", -1);

  po::variables_map vm;
  po::store( po::command_line_parser(argc, argv)
           . options(desc).positional(p).run()
           , vm
           );
  po::notify (vm);

  if (vm.count("help"))
    {
      std::cout << desc << std::endl;

      return EXIT_SUCCESS;
    }

  if (input == "-")
    {
      input = "/dev/stdin";
    }

  if (output == "-")
    {
      output = "/dev/stdout";
    }

  we::activity_t act;

  {
    std::ifstream stream (input.c_str());

    if (!stream)
      {
        throw std::runtime_error
          ("could not open file " + input + " for reading");
      }

    we::util::text_codec::decode (std::cin, act);
  }

  {
    std::ofstream stream (output.c_str());

    if (!stream)
      {
        throw std::runtime_error
          ("could not open file " + input + " for writing");
      }

    switch (tolower (key[0]))
      {
      case 'o': stream << "output:" << std::endl;
        dump (stream, act.output());
        break;
      case 'i': stream << "input:" << std::endl;
        dump (stream, act.input());
        break;
      case 'p': stream << "pending input:" << std::endl;
        dump (stream, act.pending_input());
        break;
      default:
        throw std::runtime_error ("unknown key: " + key);
      }
  }

  return EXIT_SUCCESS;
}
