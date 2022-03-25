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

#include <util-qt/forward_decl.hpp>

#include <functional>

FHG_UTIL_QT_FORWARD_DECL (class QDropEvent)
FHG_UTIL_QT_FORWARD_DECL (class QMimeData)
FHG_UTIL_QT_FORWARD_DECL (class QWidget)

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      void make_drop_target ( QWidget* widget
                            , std::function<bool (QMimeData const*)> check_accepted
                            , std::function<void (QDropEvent const*)> on_drop
                            );

      void make_drop_target ( QWidget* widget
                            , QString const& accepted_mime_type
                            , std::function<void (QDropEvent const*)> on_drop
                            );
    }
  }
}
