// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/signature.hpp>
#include <we/type/signature/cpp.hpp>
#include <we/type/signature/show.hpp>

#include <util-generic/print_exception.hpp>

#include <fstream>
#include <iostream>
#include <string>

int main()
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

  std::string const fheader {"sig_struct.hpp"};
  std::string const fimpl {"sig_op.cpp"};
  std::string const fheader_op {"sig_op.hpp"};

  std::ofstream {fheader}
    << "#pragma once\n"
    << header (p) << std::endl
    << header (l) << std::endl
    ;

  std::ofstream {fheader_op}
    << "#pragma once\n"
    << "#include \"" << fheader << "\"\n"
    << header_op (p) << std::endl
    << header_op (l) << std::endl
    ;

  std::ofstream {fimpl}
    << "#include <" << fheader << ">" << std::endl
    << "#include \"" << fheader_op << "\"\n"
    << impl (p) << std::endl
    << impl (l) << std::endl
    ;
}
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';
  return 1;
}
