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

#include <gpi-space/pc/global/itopology.hpp>

namespace gpi
{
  namespace tests
  {
    class dummy_topology : public gpi::pc::global::itopology_t
    {
    public:
      virtual void alloc ( const gpi::pc::type::segment_id_t /* segment */
                , const gpi::pc::type::handle_t /* handle */
                , const gpi::pc::type::offset_t /* offset */
                , const gpi::pc::type::size_t /* size */
                , const gpi::pc::type::size_t /* local_size */
                , const std::string & /* name */
                ) override
      {}

      virtual void free (const gpi::pc::type::handle_t) override
      {}

      virtual void add_memory ( const gpi::pc::type::segment_id_t
                     , const std::string &
                     ) override
      {}

      virtual void del_memory (const gpi::pc::type::segment_id_t) override
      {}

      virtual bool is_master () const override
      {
        return true;
      }
    };
  }
}
