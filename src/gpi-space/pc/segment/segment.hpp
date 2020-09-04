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

#include <string>
#include <boost/noncopyable.hpp>

#include <gpi-space/pc/type/segment_descriptor.hpp>

namespace gpi
{
  namespace pc
  {
    namespace segment
    {
      class segment_t : boost::noncopyable
      {
      public:
        segment_t ( std::string const & name
                  , const type::size_t sz
                  , const type::segment_id_t id = type::segment::SEG_INVAL
                  );

        ~segment_t ();

        void create (const mode_t mode = 00600);
        void open ();
        void close ();
        void unlink ();

        std::string const & name () const { return m_descriptor.name; }
        void assign_id (const type::segment_id_t);
        type::segment_id_t id () const { return m_descriptor.id; }
        type::size_t size () const { return m_descriptor.local_size; }

        type::segment::descriptor_t const & descriptor() const { return m_descriptor; }
        type::segment::descriptor_t & descriptor() { return m_descriptor; }

        template<typename T>
        T* ptr () { return (T*)ptr(); }

        void *ptr ();
        const void *ptr () const;
      private:
        gpi::pc::type::segment::descriptor_t m_descriptor;
        void *m_ptr;
      };
    }
  }
}
