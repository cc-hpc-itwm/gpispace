// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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
