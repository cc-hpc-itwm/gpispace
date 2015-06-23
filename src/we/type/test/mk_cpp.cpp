// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature.hpp>
#include <we/type/signature/show.hpp>
#include <we/type/signature/cpp.hpp>

#include <util-generic/print_exception.hpp>

#include <boost/program_options.hpp>

#include <iostream>
#include <fstream>

int main (int argc, char** argv)
try
{
  using pnet::type::signature::structure_type;
  using pnet::type::signature::structured_type;
  using pnet::type::signature::show;
  using pnet::type::signature::cpp::header;
  using pnet::type::signature::cpp::header_op;
  using pnet::type::signature::cpp::impl;

  structure_type f;
  f.push_back (std::make_pair (std::string ("x"), std::string ("float")));
  f.push_back (std::make_pair (std::string ("y"), std::string ("float")));
  structure_type ps;
  ps.push_back (std::make_pair (std::string ("p"), std::string ("point2D")));
  ps.push_back (structured_type (std::make_pair ("q", f)));
  structured_type l (std::make_pair ("line2D", ps));
  structured_type p (std::make_pair ("point2D", f));

  namespace po = boost::program_options;

  po::options_description desc ("Options");

  std::string fheader;
  std::string fimpl;
  std::string fheader_op;

  desc.add_options()
    ( "help,h"
    , "show this help message"
    )
    ( "header,H"
    , po::value<std::string>(&fheader)->default_value (fheader)
    , "filename for struct definition header"
    )
    ( "header-op,O"
    , po::value<std::string>(&fheader_op)->default_value (fheader_op)
    , "filename for operations header"
    )
    ( "impl,I"
    , po::value<std::string>(&fimpl)->default_value (fimpl)
    , "filename for operations implementation"
    );

  po::variables_map vm;
  po::store (po::parse_command_line (argc, argv, desc), vm);
  po::notify (vm);

  if (vm.count ("help"))
  {
    std::cout
      << "Print header and/or implementation for the signatures: " << std::endl
      << "    " << show (l) << " and " << std::endl
      << "    " << show (p) << std::endl << std::endl
      << desc << std::endl
      ;

    return 1;
  }

  if (!fheader.empty())
  {
    std::ofstream h (fheader.c_str());

    if (!h)
    {
      throw std::runtime_error ("Could not open " + fheader);
    }

    h << header (p) << std::endl;
    h << header (l) << std::endl;
  }

  if (!fheader_op.empty())
  {
    std::ofstream h (fheader_op.c_str());

    if (!h)
    {
      throw std::runtime_error ("Could not open " + fheader_op);
    }

    h << header_op (p) << std::endl;
    h << header_op (l) << std::endl;
  }

  if (!fimpl.empty())
  {
    std::ofstream i (fimpl.c_str());

    if (!i)
    {
      throw std::runtime_error ("Could not open " + fimpl);
    }

    if (!fheader.empty())
    {
      i << "#include <" << fheader << ">" << std::endl;
    }

    i << impl (p) << std::endl;
    i << impl (l) << std::endl;
  }
}
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';
  return 1;
}
