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

#pragma once

#include <we/type/signature.hpp>

#include <iosfwd>
#include <list>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace cpp
      {
        class header
        {
        public:
          header (structured_type const&);
          std::ostream& operator() (std::ostream&) const;
        private:
          structured_type const& _structured;
        };
        std::ostream& operator<< (std::ostream&, header const&);

        class header_signature
        {
        public:
          header_signature (signature_type const&);
          std::ostream& operator() (std::ostream&) const;
        private:
          signature_type const& _signature;
        };
        std::ostream& operator<< (std::ostream&, header_signature const&);

        class header_op
        {
        public:
          header_op (structured_type const&);
          std::ostream& operator() (std::ostream&) const;
        private:
          structured_type const& _structured;
        };
        std::ostream& operator<< (std::ostream&, header_op const&);

        class header_op_signature
        {
        public:
          header_op_signature (signature_type const&);
          std::ostream& operator() (std::ostream&) const;
        private:
          signature_type const& _signature;
        };
        std::ostream& operator<< (std::ostream&, header_op_signature const&);

        class impl
        {
        public:
          impl (structured_type const&);
          std::ostream& operator() (std::ostream&) const;
        private:
          structured_type const& _structured;
        };
        std::ostream& operator<< (std::ostream&, impl const&);

        class impl_signature
        {
        public:
          impl_signature (signature_type const&);
          std::ostream& operator() (std::ostream&) const;
        private:
          signature_type const& _signature;
        };
        std::ostream& operator<< (std::ostream&, impl_signature const&);
      }
    }
  }
}
