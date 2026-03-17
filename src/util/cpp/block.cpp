// Copyright (C) 2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/cpp/block.hpp>

#include <iostream>




      namespace gspc::util::cpp::block
      {
        open::open (gspc::util::indenter& indent)
          : _indent (indent)
          , _tag (std::nullopt)
        {}
        open::open (gspc::util::indenter& indent, std::string const& tag)
          : _indent (indent)
          , _tag (tag)
        {}
        std::ostream& open::operator() (std::ostream& os) const
        {
          return _tag
            ? os << _indent++ << *_tag << " {"
            : os << _indent++ << "{"
            ;
        }

        close::close (gspc::util::indenter& indent)
          : _indent (indent)
        {}
        std::ostream& close::operator() (std::ostream& os) const
        {
          return os << --_indent << "}";
        }
      }
