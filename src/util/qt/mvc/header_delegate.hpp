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

#include <util/qt/mvc/header_delegate.fwd.hpp>

#include <util/qt/mvc/delegating_header_view.fwd.hpp>
#include <util/qt/mvc/section_index.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        class header_delegate
        {
        public:
          virtual ~header_delegate() = default;

          virtual void paint
            (QPainter*, QRect const&, section_index const&) = 0;
          virtual QWidget* create_editor
            (QRect const&, delegating_header_view*, section_index const&) = 0;
          virtual void release_editor (section_index const&, QWidget* editor) = 0;
          virtual void update_editor (section_index, QWidget* editor) = 0;
          virtual bool can_edit_section (section_index) const = 0;
          virtual QMenu* menu_for_section (section_index) const = 0;
          virtual void wheel_event (section_index, QWheelEvent*) = 0;
        };
      }
    }
  }
}
