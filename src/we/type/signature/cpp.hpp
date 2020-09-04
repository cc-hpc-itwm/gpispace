// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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
          header (const structured_type&);
          std::ostream& operator() (std::ostream&) const;
        private:
          const structured_type& _structured;
        };
        std::ostream& operator<< (std::ostream&, const header&);

        class header_signature
        {
        public:
          header_signature (const signature_type&);
          std::ostream& operator() (std::ostream&) const;
        private:
          const signature_type& _signature;
        };
        std::ostream& operator<< (std::ostream&, const header_signature&);

        class header_op
        {
        public:
          header_op (const structured_type&);
          std::ostream& operator() (std::ostream&) const;
        private:
          const structured_type& _structured;
        };
        std::ostream& operator<< (std::ostream&, const header_op&);

        class header_op_signature
        {
        public:
          header_op_signature (const signature_type&);
          std::ostream& operator() (std::ostream&) const;
        private:
          const signature_type& _signature;
        };
        std::ostream& operator<< (std::ostream&, const header_op_signature&);

        class impl
        {
        public:
          impl (const structured_type&);
          std::ostream& operator() (std::ostream&) const;
        private:
          const structured_type& _structured;
        };
        std::ostream& operator<< (std::ostream&, const impl&);

        class impl_signature
        {
        public:
          impl_signature (const signature_type&);
          std::ostream& operator() (std::ostream&) const;
        private:
          const signature_type& _signature;
        };
        std::ostream& operator<< (std::ostream&, const impl_signature&);
      }
    }
  }
}
