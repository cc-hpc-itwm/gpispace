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

#include <map/transform_file/type.hpp>

namespace transform_file
{
  namespace
  {
    enum field {INPUT, OUTPUT, SIZE, NUM_FIELDS};
  }

  parameter from_bytearray (we::type::bytearray const& ba)
  {
    std::string const s (ba.to_string());
    unsigned long const size_header (NUM_FIELDS * sizeof (unsigned long));

    if (s.size() < size_header)
    {
      throw std::runtime_error ("Bytearray shorter than header.");
    }

    std::vector<char> data (s.begin(), s.begin() + size_header);

    unsigned long* sizes
      (static_cast<unsigned long*> (static_cast<void*> (data.data())));

    unsigned long const size_input {sizes[INPUT]};
    unsigned long const size_output {sizes[OUTPUT]};
    unsigned long const size {sizes[SIZE]};

    if (s.size() < size_header + size_input + size_output)
    {
      throw std::runtime_error ("Bytearray to short, inconsistent header");
    }

    return parameter
      ( ::boost::filesystem::path (s.substr (size_header, size_input))
      , ::boost::filesystem::path (s.substr (size_header + size_input, size_output))
      , size
      );
  }

  we::type::bytearray to_bytearray (parameter const& p)
  {
    unsigned long const size_header {NUM_FIELDS * sizeof (unsigned long)};
    unsigned long const size_input {p.input().string().size()};
    unsigned long const size_output {p.output().string().size()};

    std::vector<char> data (size_header + size_input + size_output);

    unsigned long* sizes
      (static_cast<unsigned long*> (static_cast<void*> (data.data())));

    sizes[INPUT] = size_input;
    sizes[OUTPUT] = size_output;
    sizes[SIZE] = p.size();

    std::copy ( p.input().string().begin(), p.input().string().end()
              , data.data() + size_header
              );
    std::copy ( p.output().string().begin(), p.output().string().end()
              , data.data() + size_header + size_input
              );

    return we::type::bytearray
      (data.data(), size_header + size_input + size_output);
  }
}
