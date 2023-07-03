// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

          header_delegate() = default;
          header_delegate (header_delegate const&) = delete;
          header_delegate& operator= (header_delegate const&) = delete;
          header_delegate (header_delegate&&) = delete;
          header_delegate& operator= (header_delegate&&) = delete;

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
