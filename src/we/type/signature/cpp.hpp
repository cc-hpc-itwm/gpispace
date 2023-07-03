// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
