// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <boost/serialization/nvp.hpp>

namespace we
{
  namespace type
  {
    template<typename Archive>
      void ModuleCall::serialize (Archive& ar, unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP (module_);
      ar & BOOST_SERIALIZATION_NVP (function_);
      ar & BOOST_SERIALIZATION_NVP (_memory_buffers);
      ar & BOOST_SERIALIZATION_NVP (_memory_gets);
      ar & BOOST_SERIALIZATION_NVP (_memory_puts);
      ar & BOOST_SERIALIZATION_NVP (_require_function_unloads_without_rest);
      ar & BOOST_SERIALIZATION_NVP (_require_module_unloads_without_rest);
    }
  }
}
