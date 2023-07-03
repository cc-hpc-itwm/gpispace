// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtGui/QPainter>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      struct painter_state_saver
      {
        painter_state_saver (QPainter* const painter)
          : _painter (painter)
        {
          _painter->save();
        }
        ~painter_state_saver()
        {
          _painter->restore();
        }

        painter_state_saver (painter_state_saver const&) = delete;
        painter_state_saver& operator= (painter_state_saver const&) = delete;
        painter_state_saver (painter_state_saver&&) = delete;
        painter_state_saver& operator= (painter_state_saver&&) = delete;

      private:
        QPainter* const _painter;
      };
    }
  }
}
